#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// we need it to check if file is in arc format
const char signature[8] = "arcfile";

// arc header
typedef struct {
    char signature[8];
    int n;
} AHeader;

// file header
typedef struct {
    int name_size;
    long data_size;
} FHeader;

int read_and_check_arc_header(FILE *arc_file, AHeader *ptr)
{
    if (fread(ptr, sizeof(AHeader), 1, arc_file) != 1)
    {
        printf("read error\n");
        return -1;
    }

    if(strcmp(ptr->signature, signature) != 0)
    {
        printf("not arc file\n");
        return -1;
    }

    return 0;
}

int write_arc_header(FILE *arc_file, int n)
{
    AHeader ah;

    strcpy(ah.signature, signature);
    ah.n = n;

    if (fwrite(&ah, sizeof(ah), 1, arc_file) != 1)
    {
        printf("can't write into arc\n");
        return -1;
    }

    return 0;
}

void print_file_header(FHeader *fh)
{
    printf("fh: %d %ld\n", fh->name_size, fh->data_size);
}

int write_file(char *file_name, char *data, long data_size)
{
    printf("creating file %s\n", file_name);

    FILE *f = fopen(file_name, "wb");
    if(f == NULL)
    {
        printf("can't create file %s\n", file_name);
        return -1;
    }

    if (fwrite(data, data_size, 1, f) != 1)
    {
        printf("can't write into file %s\n", file_name);
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

int get_file_size(char* file_name, long* file_size)
{
    FILE *f = fopen(file_name, "rb");

    if(f == NULL)
    {
        printf("can't open file %s\n", file_name);
        return -1;
    }

    // go to end of file
    if (fseek(f, 0, SEEK_END) != 0)
    {
        printf("read error\n");
        fclose(f);
        return -1;
    }

    // get file size
    *file_size = ftell(f);

    fclose(f);
    return 0;
}

void create_arc(char *arc_name, char **file_names, int n)
{
    FHeader fh;
    int err = 0;

    char *file_data = NULL;

    FILE *arc_file = fopen(arc_name, "wb");
    if(arc_file == NULL)
    {
        printf("can't create archive file\n");
        return;
    }

    if(write_arc_header(arc_file, n) != 0)
    {
        fclose(arc_file);
        remove(arc_name);
        return;
    }

    // adding files to arc
    for (int i = 0; i < n; i++)
    {
        //create file header
        fh.name_size = strlen(file_names[i])+1;
        if (get_file_size(file_names[i], &fh.data_size) != 0)
        {
            err = -1;
            break;
        }
        //print_file_header(&fh);

        // read all data from file

        FILE *f = fopen(file_names[i], "rb");
        if(f == NULL)
        {
            printf("can't open file %s\n", file_names[i]);
            err = -1;
            break;
        }

        file_data = malloc(fh.data_size);
        if(file_data == NULL)
        {
            printf("can't allocate %ld\n", fh.data_size);
            fclose(f);
            err = -1;
            break;
        }

        if (fread(file_data, fh.data_size, 1, f) != 1)
        {
            printf("read error\n");
            fclose(f);
            err = -1;
            break;
        }

        fclose(f);

        // writing into arc file

        if (fwrite(&fh, sizeof(fh), 1, arc_file) != 1)
        {
            printf("write error\n");
            err = -1;
            break;
        }

        if (fwrite(file_names[i], fh.name_size, 1, arc_file) != 1)
        {
            printf("write error\n");
            err = -1;
            break;
        }
        if (fwrite(file_data, fh.data_size, 1, arc_file) != 1)
        {
            printf("write error\n");
            err = -1;
            break;
        }

        free(file_data);
        file_data = NULL;
    }

    if(file_data != NULL)
        free(file_data);
    fclose(arc_file);
    if (err != 0)
        remove(arc_name);
}

void extract_arc(char* arc_name)
{
    AHeader ah;
    FHeader fh;

    char *file_name = NULL;
    char *file_data = NULL;

    FILE *arc_file = fopen(arc_name, "rb");
    if(arc_file == NULL)
    {
        printf("can't open archive file\n");
        return;
    }

    if (read_and_check_arc_header(arc_file, &ah) != 0)
    {
        fclose(arc_file);
        return;
    }

    // extracting files

    for (int i = 0; i < ah.n; i++)
    {
        if (fread(&fh, sizeof(fh), 1, arc_file) != 1)
        {
            printf("read error\n");
            break;
        }

        // allocating memory for file_data and file_name

        file_name = malloc(fh.name_size);
        if(file_name == NULL)
        {
            printf("can't allocate %d\n", fh.name_size);
            break;
        }

        file_data = malloc(fh.data_size);
        if(file_data == NULL)
        {
            printf("can't allocate %ld\n", fh.data_size);
            break;
        }

        // extracting file_data and file_name

        if (fread(file_name, fh.name_size, 1, arc_file) != 1)
        {
            printf("read error\n");
            break;
        }

        if (fread(file_data, fh.data_size, 1, arc_file) != 1)
        {
            printf("read error\n");
            break;
        }

        // creating extracted file

        if (write_file(file_name, file_data, fh.data_size) != 0)
        {
            break;
        }

        free(file_name);
        file_name = NULL;
        free(file_data);
        file_data = NULL;
    }

    if(file_name != NULL)
        free(file_name);

    if(file_data != NULL)
        free(file_data);

    fclose(arc_file);
}

void list_arc(char* arc_name)
{
    AHeader ah;
    FHeader fh;

    char *file_name = NULL;

    FILE *arc_file = fopen(arc_name, "rb");
    if(arc_file == NULL)
    {
        printf("can't open archive file\n");
        return;
    }

    if (read_and_check_arc_header(arc_file, &ah) != 0)
    {
        fclose(arc_file);
        return;
    }

    for (int i = 0; i < ah.n; i++)
    {
        if (fread(&fh, sizeof(fh), 1, arc_file) != 1)
        {
            printf("read error\n");
            break;
        }
        //print_file_header(&fh);

        // Print name of file
        char *file_name = malloc(fh.name_size);
        if(file_name == NULL)
        {
            printf("can't allocate %d\n", fh.name_size);
            break;
        }

        if (fread(file_name, fh.name_size, 1, arc_file) != 1)
        {
            printf("read error\n");
            break;
        }

        printf("%s\n", file_name);
        free(file_name);
        file_name = NULL;

        if (fseek(arc_file, fh.data_size, SEEK_CUR) != 0)
        {
            printf("read error\n");
            break;
        }
   }

    if(file_name != NULL)
        free(file_name);

    fclose(arc_file);
}

// --file FILE --create FILE1 FILE2 ... FILEN
// --file FILE --extract
// --file FILE --list

int main(int argc, char *argv[])
{
    if ((argc > 4) && (strcmp(argv[1], "--file") == 0) && (strcmp(argv[3], "--create") == 0))
    {
        for (int i = 4; i < argc; i++)
        {
            if(strcmp(argv[2], argv[i]) == 0)
            {
                printf("error - file name is equal to arc name..");
                return;
            }
        }
        create_arc(argv[2], &argv[4], argc-4);
    }
    else if ((argc == 4) && (strcmp(argv[1], "--file") == 0) && (strcmp(argv[3], "--extract") == 0))
        extract_arc(argv[2]);
    else if ((argc == 4) && (strcmp(argv[1], "--file") == 0) && (strcmp(argv[3], "--list") == 0))
        list_arc(argv[2]);
    else
        printf("Invalid args\n");

    return 0;
}