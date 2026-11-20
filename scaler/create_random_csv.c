#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define FILES_COUNT 5000

int main()
{
    int x, rows, columns, r, c;
    char file_name[51];

    FILE *file;
    int min, max;
    int value;

    srand(time(NULL));
    for (x = 1; x <= FILES_COUNT; ++x)
    {
        sprintf(file_name, "source/data_%d.csv", x);
        file = fopen(file_name, "wb");

        min = 3;
        max = 45;

        columns = rand() % (max - min) + min;
        min = 1000;
        max = 9000;
        rows = rand() % (max - min) + min;
        min = 1;
        max = 1000000;
        for (r = 1; r <= rows; ++r)
        {
            for (c = 1; c < columns; ++c)
            {
                value = rand() % (max - min) + min;
                fprintf(file, "%d,", value);
            }
            value = rand() % (max - min) + min;
            fprintf(file, "%d\n", value);
        }

        fclose(file);

        printf("File %s created with %d rows and %d columns\n", file_name, rows, columns);
    }
    return 0;
}