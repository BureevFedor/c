#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char data[14];
    unsigned int bfSize; //file size in bytes
    unsigned int bfOffBits; // offset pixels data from the beginning of the file
} BitMapHeader;

typedef struct {
    unsigned int data_size; //all_data
    unsigned char* data;
    unsigned int size; //size of byte info header
    unsigned int height;
    unsigned int width;
} BitInfoHeader;

typedef struct {
    unsigned int data_size;
    unsigned char* data;
    unsigned char* elements;
    unsigned int height;
    unsigned int width;
    unsigned int act_width;
} Info;

BitMapHeader header1;
BitInfoHeader header2;
Info cur_table;
Info next_table;

int read_file(char* filename)
{
    FILE *file = fopen(filename, "rb");

    if(BitMapHeader_read(file, &header1) != 0)
    {
        printf("can't read bitmapheader\n");
        fclose(file);
        return -1;
    }

    if(BitInfoHeader_read(file, &header2, header1.bfOffBits-14) != 0)
    {
        printf("can't read bitinfoheader\n");
        fclose(file);
        return -1;
    }

    if(Info_init(&cur_table, header2.width, header2.height, header1.bfSize - header1.bfOffBits) != 0)
        return -1;

    if(Info_read(file, &cur_table) != 0)
    {
        printf("can't read info\n");
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

int write_file(char* filename)
{
    FILE *file = fopen(filename, "wb");

    if(BitMapHeader_write(file, &header1) != 0)
    {
        printf("can't write bitmapheader\n");
        fclose(file);
        return -1;
    }
    if(BitInfoHeader_write(file, &header2) != 0)
    {
        printf("can't write bitinfoheader\n");
        fclose(file);
        return -1;
    }
    if(Info_write(file, &next_table) != 0)
    {
        printf("can't write info\n");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

int BitMapHeader_read(FILE* file, BitMapHeader* h1)
{
    if(fread(h1->data, 14, 1, file) != 1)
        return -1;
    if((h1->data[0] != 'B') || (h1->data[1] != 'M'))
    {
        printf("not a bmp file\n");
        return -1;
    }

    h1->bfSize = (unsigned int)(h1->data[2]) + ((unsigned int)(h1->data[3]) << 8) + ((unsigned int)(h1->data[4]) << 16) + ((unsigned int)(h1->data[5]) << 24);
    h1->bfOffBits = (unsigned int)(h1->data[10]) + ((unsigned int)(h1->data[11]) << 8) + ((unsigned int)(h1->data[12]) << 16) + ((unsigned int)(h1->data[13]) << 24);

    //printf("bitmapheader: bfsize=%u, bfoffbits=%u\n", h1->bfSize, h1->bfOffBits);
    return 0;
}

int BitMapHeader_write(FILE* file, BitMapHeader* h1)
{
    if(fwrite(h1->data, 14, 1, file) != 1)
        return -1;
    return 0;
}

int BitInfoHeader_read(FILE* file, BitInfoHeader* h2, unsigned int info_size)
{
    h2->data_size = info_size;
    h2->data = malloc(info_size);
    if(h2->data == NULL)
    {
        printf("can't allocate memory\n");
        return -1;
    }

    if(fread(h2->data, h2->data_size, 1, file) != 1)
        return -1;

    h2->size = (unsigned int)(h2->data[0]) + ((unsigned int)(h2->data[1]) << 8) + ((unsigned int)(h2->data[2]) << 16) + ((unsigned int)(h2->data[3]) << 24);
    //different versions:
    if(h2->size == 12)
    {
        h2->width = (unsigned int)(h2->data[4]) + ((unsigned int)(h2->data[5]) << 8);
        h2->height = (unsigned int)(h2->data[6]) + ((unsigned int)(h2->data[7]) << 8);
    }
    else
    {
        h2->width = (unsigned int)(h2->data[4]) + ((unsigned int)(h2->data[5]) << 8) + ((unsigned int)(h2->data[6]) << 16) + ((unsigned int)(h2->data[7]) << 24);
        h2->height = (unsigned int)(h2->data[8]) + ((unsigned int)(h2->data[9]) << 8) + ((unsigned int)(h2->data[10]) << 16) + ((unsigned int)(h2->data[11]) << 24);
    }

    //printf("bitinfoheader: data_size=%u, size=%u, width=%u, height=%u\n", h2->data_size, h2->size, h2->width, h2->height);

    return 0;
}

int BitInfoHeader_write(FILE* file, BitInfoHeader* h2)
{
    if(fwrite(h2->data, h2->data_size, 1, file) != 1)
        return -1;
    return 0;
}

void Info_print(Info* table)
{
    for(int i = 0; i < table->height; i++)
    {
        for(int j = 0; j < table->width; j++)
        {
            if(table->elements[i * table->width + j] == 1)
                printf("8");
            else
                printf(".");
        }
        printf("\n");
    }
}

int Info_init(Info* table, unsigned int width, unsigned int height, unsigned int info_size)
{
    table->width = width;
    table->height = height;
    table->data_size = info_size;
    table->act_width = width + ((width%32 == 0) ? 0 : (32 - width%32));

    table->data = calloc(1, info_size);
    if(table->data == NULL)
    {
        printf("can't allocate memory\n");
        return -1;
    }

    table->elements = calloc(1, width * height);
    if(table->elements == NULL)
    {
        printf("can't allocate memory\n");
        return -1;
    }

    //printf("data_size=%u, width=%u, height=%u\n", table->data_size, table->width, table->height);
    return 0;
}

int Info_read(FILE* file, Info* table)
{
    unsigned int k = 0; //k - number of bit with padding

    if(fread(table->data, table->data_size, 1, file) != 1)
        return -1;

    for(int i = 0; i < table->height; i++)
    {
        for(int j = 0; j < table->width; j++)
        {
            k = (table->height - 1 - i) * table->act_width + j;
            table->elements[i * table->width + j] = ((((table->data[k / 8]) >> (7 - (k % 8))) & 0x01) == 0) ? 1 : 0;
             // Get a byte, move it to the right so we get the bit that we need, then 0 all other bits
        }
    }

    return 0;
}

int Info_write(FILE* file, Info* table)
{
    unsigned int k = 0;

    memset(table->data, 0, table->data_size);

    for(int i = 0; i < table->height; i++)
    {
        for(int j = 0; j < table->width; j++)
        {
            if(table->elements[i * table->width + j] == 0)
            {
                k = (table->height - 1 - i) * table->act_width + j;
                table->data[k/8] = table->data[k/8] | (0x01 << (7 - (k % 8)));
            }
        }
    }

    if(fwrite(table->data, table->data_size, 1, file) != 1)
        return -1;

    return 0;
}

unsigned int alive(int i, int j, Info* table)
{
    int i1 = ((i < 0) ? (table->height - 1) : ((i == table->height) ? 0 : i));
    int j1 =((j < 0) ? (table->width - 1) : ((j == table->width) ? 0 : j));

    //printf("i1 = %d, j1 = %d, i = %d, j = %d, sum = %d\n", i1, j1, i, j, (i1 * table->width) + j1);
    return table->elements[(i1 * table->width) + j1];
}

int next_iteration(Info* table1, Info* table2)
{
    memset(table2->elements, 0, table2->width * table2->height);

    unsigned int alive_cnt = 0;
    unsigned int width = table1->width;
    unsigned int height = table1->height;

    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
        {
            int k = i * table1->width + j;

            alive_cnt =
                alive(i-1, j-1, table1) +
                alive(i-1, j, table1) +
                alive(i-1, j+1, table1) +
                alive(i, j-1,table1) +
                alive(i, j+1, table1) +
                alive(i+1, j-1, table1) +
                alive(i+1, j, table1) +
                alive(i+1, j+1, table1);

            if(alive_cnt == 3)
            {
                if(table1->elements[k] == 0)
                    table2->elements[k] = 1;
            }
            else if(alive_cnt == 2)
            {
                if(table1->elements[k] == 1)
                    table2->elements[k] = 1;
            }
        }

    return 0;
}

// 1 - diff, 0 - same
int table_cmp(Info* table1, Info* table2)
{
    for(int i = 0; i < table1->height * table1->width; i++)
        if(table1->elements[i] != table2->elements[i])
                return 1;
    return 0;
}

int main(int argc, char *argv[])
{
    char output_file[100];
    char* filename = NULL;
    char* dir = NULL;
    int max_iter = 0;
    int freq = 1;

    for(int i = 1; i < (argc-1); i += 2)
    {
        if(strcmp(argv[i], "--input") == 0)
            filename = argv[i+1];
        else if(strcmp(argv[i], "--output") == 0)
            dir = argv[i+1];
        else if(strcmp(argv[i], "--max_iter") == 0)
            max_iter = atoi(argv[i+1]);
        else if(strcmp(argv[i], "--dump_freq") == 0)
            freq = atoi(argv[i+1]);
        else
        {
            printf("invalid arguments\ncorrect arguments: --input input_file.bmp --output dir_name --max_iter N --dump_freq N\n");
            return -1;
        }
    }

    if((filename == NULL) || (dir == NULL))
    {
        printf("invalid arguments\ncorrect arguments: --input input_file.bmp --output dir_name --max_iter N --dump_freq N\n");
        return -1;
    }

    //printf("filename = %s, dir = %s, max_iter = %d, dump_freq = %d", filename, dir, max_iter, freq);

    if(read_file(filename) != 0)
        goto clean_data;

    //Info_print(&cur_table);

    if(Info_init(&next_table, cur_table.width, cur_table.height, cur_table.data_size) != 0)
        goto clean_data;

    int num_iter = 1;

    while((max_iter == 0) || ((max_iter != 0) && (num_iter <= max_iter)))
    {
        next_iteration(&cur_table, &next_table);

        if(table_cmp(&cur_table, &next_table) == 0)
        {
            printf("life is freezed\n");
            break;
        }

        if(num_iter % freq == 0)
        {
            sprintf(output_file, "%s\\%d.bmp", dir, num_iter);
            //printf("%s\n", output_file);
            write_file(output_file);
        }

        memcpy(cur_table.elements, next_table.elements, cur_table.width * cur_table.height);

        num_iter++;
    }


clean_data:
    if(header2.data != NULL)
        free(header2.data);
    if(cur_table.data != NULL)
        free(cur_table.data);
    if(cur_table.elements != NULL)
        free(cur_table.elements);
    if(next_table.data != NULL)
        free(next_table.data);
    if(next_table.elements != NULL)
        free(next_table.elements);

    return 0;
}
