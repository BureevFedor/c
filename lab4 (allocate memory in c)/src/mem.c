#define _DEFAULT_SOURCE

#include <unistd.h>

#include "mem_internals.h"
#include "mem.h"
#include "util.h"

void debug_block(struct block_header* b, const char* fmt, ... );
void debug(const char* fmt, ... );

extern inline block_size size_from_capacity( block_capacity cap );
extern inline block_capacity capacity_from_size( block_size sz );

static bool block_is_big_enough( size_t query, struct block_header* block ) 
{ 
  return block->capacity.bytes >= query; 
}
static size_t pages_count (size_t mem) 
{
  return mem / getpagesize() + ((mem % getpagesize()) > 0); 
}
static size_t round_pages (size_t mem)
{ 
  return getpagesize() * pages_count( mem );
}

static void block_init( void* addr, block_size block_sz, void* next ) {
  *((struct block_header*)addr) = (struct block_header) {
    .next = next,
    .capacity = capacity_from_size(block_sz),
    .is_free = true
  };
}

static size_t region_actual_size(size_t query)
{ 
  return size_max( round_pages(query), REGION_MIN_SIZE ); 
}

extern inline bool region_is_invalid(const struct region* r);

static void* map_pages(void const* addr, size_t length, int additional_flags) 
{
  return mmap( (void*) addr, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | additional_flags , -1, 0 );
}

/* аллоцировать регион памяти и инициализировать его блоком */
static struct region alloc_region  (void const * addr, size_t query) 
{
    size_t required_size = size_from_capacity((block_capacity) {.bytes=query}).bytes;
    size_t actual_size = region_actual_size(required_size);

    void* region_addr = map_pages(addr,actual_size, MAP_FIXED);
    if(region_addr == MAP_FAILED) 
    {
        region_addr = map_pages(addr, actual_size, 0);
        if(region_addr == MAP_FAILED) {
          return REGION_INVALID;
        }
    }
    block_init(region_addr, (block_size){.bytes = actual_size}, NULL);
    return (struct region) {.addr = region_addr, .size = actual_size, .extends = region_addr == addr};
}

static void* block_after(struct block_header const* block);

void* heap_init(size_t initial) 
{
  const struct region region = alloc_region(HEAP_START, initial);
  if ( region_is_invalid(&region) ) return NULL;
  if( region.addr != HEAP_START ) return NULL;
  return region.addr;
}

/*  освободить всю память, выделенную под кучу */
void heap_term()
{
  struct block_header* start = (struct block_header*) HEAP_START;
  if(!start) return;

  while(start){
      struct block_header* next_block = start->next;
      size_t size = size_from_capacity(start->capacity).bytes;
      while(next_block && block_after(start)==next_block){
          size_t size_nb = size_from_capacity(next_block->capacity).bytes;
          size += size_nb;
          start->capacity.bytes += size_nb;

          start->next = next_block->next;
          next_block = start->next;
      }
      if(munmap(start, size) != 0) return ;
      start = next_block;
  }
}

#define BLOCK_MIN_CAPACITY 24

/*  --- Разделение блоков (если найденный свободный блок слишком большой )--- */
static bool block_splittable( struct block_header* restrict block, size_t query) 
{
  return block-> is_free && query + offsetof( struct block_header, contents ) + BLOCK_MIN_CAPACITY <= block->capacity.bytes;
}
static bool split_if_too_big( struct block_header* block, size_t query ) 
{
    if(!block || !(block_splittable(block, query))) return false;

    void* new_block = block->contents + query;
    block_size new_bz = (block_size){block->capacity.bytes - query};

    block_init(new_block, new_bz, block->next);

    block->capacity.bytes = query;
    block->next = new_block;

    return true;
}


/*  --- Слияние соседних свободных блоков --- */
static void* block_after( struct block_header const* block ) 
{
  return  (void*) (block->contents + block->capacity.bytes);
}
static bool blocks_continuous ( struct block_header const* fst, struct block_header const* snd ) 
{
  return (void*)snd == block_after(fst);
}
// готовая конструкция для проверки 2-х блоков на соединяемость
static bool mergeable(struct block_header const* restrict fst, struct block_header const* restrict snd) 
{
  return fst->is_free && snd->is_free && blocks_continuous( fst, snd ) ;
}
static bool try_merge_with_next( struct block_header* block ) 
{
    if(!block || !block->next) return false;

	struct block_header* tmp = block->next;
    if(mergeable(block, tmp)){
        block->next = tmp->next;
        block->capacity.bytes += size_from_capacity(tmp->capacity).bytes;
        return true;
    }
    return false;
}

/*  --- ... ecли размера кучи хватает --- */
struct block_search_result 
{
  enum {BSR_FOUND_GOOD_BLOCK, BSR_REACHED_END_NOT_FOUND, BSR_CORRUPTED} type;
  struct block_header* block;
};

static struct block_search_result find_good_or_last  ( struct block_header* restrict block, size_t sz ) 
{
    struct block_search_result result = {.type=BSR_FOUND_GOOD_BLOCK, .block=block};
    struct block_header* prev_block = NULL;

    while(block){
        result.block = block;

        while(true){
            if(!try_merge_with_next(block))
                break;
        }
        if(block->is_free && block_is_big_enough(sz, block))
            return result;

        prev_block = block;
        block = block->next;
    }

    result.type = BSR_REACHED_END_NOT_FOUND;
    result.block = prev_block;
    return result;
}

/*  Попробовать выделить память в куче начиная с блока `block` не пытаясь расширить кучу
 Можно переиспользовать как только кучу расширили. */
static struct block_search_result try_memalloc_existing ( size_t query, struct block_header* block ) 
{
    struct block_search_result result = find_good_or_last(block, query);
    if(result.type == BSR_FOUND_GOOD_BLOCK){
        split_if_too_big(result.block, query);
            result.block->is_free = false;
    }
    return result;
}

static struct block_header* grow_heap( struct block_header* restrict last, size_t query ) {
    void* required_addr = block_after(last);
    struct region region = alloc_region(required_addr, query);

    if( region_is_invalid(&region) ) {
        return NULL;
    }
    if(region.addr == required_addr && last->is_free && region.extends) {
        last->capacity.bytes += region.size;
        return last;
    }
    last->next = region.addr;
    return region.addr;
}

/*  Реализует основную логику malloc и возвращает заголовок выделенного блока */
static struct block_header* memalloc( size_t query, struct block_header* heap_start) {
    query = size_max(query, BLOCK_MIN_CAPACITY);

    struct block_search_result result = try_memalloc_existing(query, heap_start);
    if(result.type == BSR_FOUND_GOOD_BLOCK)
        return result.block;

    else if(result.type == BSR_REACHED_END_NOT_FOUND) {
        struct block_header* allocated_block = grow_heap(result.block, query);

        if(!allocated_block) 
			return NULL;

        split_if_too_big(allocated_block, query);
        allocated_block->is_free = false;
        return allocated_block;
    }
        
    return NULL;
}

void* _malloc(size_t query) 
{
  struct block_header* const addr = memalloc(query, (struct block_header*) HEAP_START);
  if (addr) {
    return addr->contents;
  }
  
  return NULL;
}

static struct block_header* block_get_header(void* contents) 
{
  return (struct block_header*) (((uint8_t*)contents) - offsetof(struct block_header, contents));
}

void _free( void* mem ) 
{
  if (!mem) 
    return ;
    
  struct block_header* header = block_get_header( mem );
  header->is_free = true;
  while(true) {
	if(!(header->next) || !try_merge_with_next(header))
		break;
  }
}