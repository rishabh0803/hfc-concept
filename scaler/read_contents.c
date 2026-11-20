#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #define BUFFER_SIZE 1024*1024
#define BUFFER_SIZE 1024
int main()
{
    char file_name[51];
    FILE *file;
    unsigned char *buffer;
    size_t read_count;
    int i;

    buffer = (unsigned char *)malloc(BUFFER_SIZE);
    if (buffer == NULL)
    {
        printf("Low memory\n");
        return 1;
    }
    strcpy(file_name, "source/data_1.csv");

    file = fopen(file_name, "rb");
    if (file == NULL)
    {
        printf("Unable to access %s\n", file_name);
        return 1;
    }

    while (1)
    {
        read_count = fread(buffer, 1, BUFFER_SIZE, file);
        if (read_count == 0)
            break;
        printf("Bytes extracted %lu\n", read_count);
        // for(i=0;i<read_count;++i) printf("%c",buffer[i]);
    }

    fclose(file);
    free(buffer);
    return 0;
}