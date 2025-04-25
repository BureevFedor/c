#include <assert.h>
#include <stdio.h>

#include "tests.h"
#include "mem.h"
#include "mem_internals.h"

#define HEAP_SIZE 8000

void run_test_1() {
	printf("\nTest 1 (allocate 1 block)\n");

	void *heap = heap_init(HEAP_SIZE);
	assert(heap && "heap_init() failed");
	printf("\nInitial heap status:\n");
	debug_heap(stdout, heap);

	void *ptr = _malloc(25);
	assert(ptr && "_malloc() failed");
	printf("\nHeap after malloc:\n");
	debug_heap(stdout, heap);

	_free(ptr);
	heap_term();

	printf("\n////////\n");
}

void run_test_2() {
        printf("\nTest 2 (allocate 3 blocks, delete 1)\n");

        void *heap = heap_init(HEAP_SIZE);
        assert(heap && "heap_init() failed");
        printf("\nInitial heap status:\n");
        debug_heap(stdout, heap);

        void *ptr1 = _malloc(25);
        assert(ptr1 && "_malloc() failed");

        void *ptr2 = _malloc(125);
        assert(ptr2 && "_malloc() failed");

        void *ptr3 = _malloc(40);
        assert(ptr3 && "_malloc() failed");

        printf("\nHeap after allocation of 3 blocks:\n");
        debug_heap(stdout, heap);

        _free(ptr2);
        printf("\nHeap after freeing second block:\n");
        debug_heap(stdout, heap);

        _free(ptr1);
        _free(ptr3);
        heap_term();

	printf("\n////////\n");
}

void run_test_3() {
	printf("\nTest 3\n");

        void *heap = heap_init(HEAP_SIZE);
        assert(heap && "heap_init() failed");
        printf("\nInitial heap status:\n");
        debug_heap(stdout, heap);

        void *ptr1 = _malloc(100);
        assert(ptr1 && "_malloc() failed");

        void *ptr2 = _malloc(24);
        assert(ptr2 && "_malloc() failed");

        void *ptr3 = _malloc(48);
        assert(ptr3 && "_malloc() failed");

        printf("\nHeap after allocation of 3 blocks:\n");
        debug_heap(stdout, heap);

        _free(ptr3);
        printf("\nHeap after freeing the third block:\n");
        debug_heap(stdout, heap);

        _free(ptr2);
        printf("\nHeap after freeing the second block:\n");
        debug_heap(stdout, heap);

        _free(ptr1);
        heap_term();

	printf("\n////////\n");
}

void run_test_4() {
	printf("\nTest 4\n");

        void *heap = heap_init(HEAP_SIZE);
        assert(heap && "heap_init() failed");
        printf("\nInitial heap status:\n");
        debug_heap(stdout, heap);

        void *ptr1 = _malloc(8000);
        assert(ptr1 && "_malloc() failed");

        printf("\nHeap after the memory has almost run out:\n");
        debug_heap(stdout, heap);

        void *ptr2 = _malloc(HEAP_SIZE);
        assert(ptr2 && "_malloc() failed");
        printf("\nHeap extended after new huge allocation:\n");
        debug_heap(stdout, heap);

        _free(ptr2);
        _free(ptr1);
        heap_term();

	printf("\n////////\n");
}


void run_tests()
{
	run_test_1();
	run_test_2();
	run_test_3();
	run_test_4();
}
