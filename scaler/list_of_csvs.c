#include <stdio.h>
#include <dirent.h>
#include <string.h>

int main()
{
    DIR *directory;
    struct dirent *directory_entry;
    char *ptr;

    directory = opendir("source");
    if (directory == NULL)
    {
        printf("Unable to access source directory\n");
        return 1;
    }

    while (1)
    {
        directory_entry = readdir(directory);
        if (directory_entry == NULL)
            break;

        ptr = strrchr(directory_entry->d_name, '.');
        if (ptr && strcmp(ptr, ".csv") == 0)
        {
            printf("%s\n", directory_entry->d_name);
        }
    }
    closedir(directory);
    return 0;
}