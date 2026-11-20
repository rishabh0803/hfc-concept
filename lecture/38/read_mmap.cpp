#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h> // for functions related to memory management, right now (mmap)
#include <chrono>

using namespace std;
using namespace std::chrono;

int main()
{
    auto start = high_resolution_clock::now();
    struct stat s;
    stat("test.data", &s);
    size_t file_size = s.st_size;
    size_t page_size = sysconf(_SC_PAGESIZE);
    cout << "File size : " << file_size << endl;
    cout << "System page size : " << page_size << endl;

    int file_descriptor;
    file_descriptor = open("test.data", O_RDONLY);

    off_t offset;
    size_t chunk_size = page_size;
    size_t how_many_bytes_to_map;
    how_many_bytes_to_map = chunk_size;
    offset = 0;

    char c;
    int i;
    char *ptr;
    while (offset < file_size)
    {
        if ((offset + (off_t)how_many_bytes_to_map) > file_size)
            how_many_bytes_to_map = file_size - offset;
        ptr = (char *)mmap(NULL, how_many_bytes_to_map, PROT_READ, MAP_PRIVATE, file_descriptor, offset);
        if (ptr == MAP_FAILED)
        {
            cout << "Unable to map" << endl;
            return 1;
        }
        for (i = 0; i < how_many_bytes_to_map; ++i)
            c = ptr[i];
        munmap(ptr, how_many_bytes_to_map);
        offset += how_many_bytes_to_map;
    }

    close(file_descriptor);

    auto end = high_resolution_clock::now();
    auto d = duration_cast<milliseconds>(end - start);
    cout << d.count() << " milliseconds" << endl;

    return 0;
}