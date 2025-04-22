#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char str[50];

int lines_cnt(char* filename);
long bytes_cnt(char* filename);
int words_cnt(char* filename);

int main(int argc, char *argv[])
{
    //FILE *file;

    if (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--lines") == 0)
         printf("%d lines\n", lines_cnt(argv[2]));
    else if(strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--bytes") == 0)
        printf("%d bytes\n", bytes_cnt(argv[2]));
    else if(strcmp(argv[1], "-w") == 0 || strcmp(argv[1], "--words") == 0)
        printf("%d words\n", words_cnt(argv[2]));
    else if(strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--all") == 0)
    {
        printf("%d lines\n", lines_cnt(argv[2]));
        printf("%d bytes\n", bytes_cnt(argv[2]));
        printf("%d words\n", words_cnt(argv[2]));
    }
    else
        printf("invalid arguments\n");

    return 0;
}

int lines_cnt(char* filename)
{
    FILE *file = fopen(filename, "r");

    int cnt = 0;
    char* estr;

    while(!feof(file))
    {
        if(fgets(str, sizeof(str), file))
            cnt++;
    }

    fclose(file);
    return cnt;
}

long bytes_cnt(char* filename)
{
    FILE *file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    long cnt = ftell(file);

    fclose(file);
    return cnt;
}

int words_cnt(char* filename)
{
    FILE *file = fopen(filename, "r");
    int cnt = 0;
    char word[100];
    int m;

    while (!feof(file))
    {
        m = fscanf(file, "%s", word);
        if(m == 1)
        {
            //printf("%i %s\n", m, word);
            cnt++;
        }
    }

    fclose(file);
    return cnt;
}
