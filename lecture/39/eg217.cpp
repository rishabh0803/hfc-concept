#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>

using namespace std;

int main()
{
    const char *data = "ABCDEFGHIJ";
    int fd;
    fd = open("xyz.abc", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1)
    {
        cout << "Unable to create xyz.abc" << endl;
        return 1;
    }

    if (ftruncate(fd, 10) == -1)
    {
        cout << "Unable to setup size of xyz.abc" << endl;
        close(fd);
        return 1;
    }

    // MAP_SHARED required of mapping for write operation, // otherwise kernel space memory contents will not be flushed to disk
    char *ptr = (char *)mmap(NULL, 10, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memcpy(ptr, data, 10);
    munmap(ptr, 10);

    close(fd);

    cout << "File created" << endl;

    return 0;
}