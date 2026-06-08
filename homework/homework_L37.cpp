// Program to read a file using pread and chunk wise in parallel using multiple threads.

#include <iostream>
#include <stdio.h>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <thread>
#include <vector>

using namespace std;

int main()
{
    char *buffer;
    // buffer=new char[8*1024];
    buffer = new char[4 * 1024 * 1024];
    int file_descriptor;
    int read_count;

    int number_of_cores = thread::hardware_concurrency();
    int number_of_threads = number_of_cores - 1;

    // read file size using stat
    struct stat file_stat;
    stat("test.data", &file_stat);
    uint64_t file_size = file_stat.st_size;

    // calculate number of chunks
    uint64_t chunk_size = 4 * 1024 * 1024;
    uint64_t number_of_chunks = file_size / chunk_size;
    if (file_size % chunk_size != 0)
        number_of_chunks++;

    cout << "Number of chunks: " << number_of_chunks << endl;

    // how many chunks each thread will read
    uint64_t chunks_per_thread = number_of_chunks / number_of_threads;
    uint64_t remaining_chunks = number_of_chunks % number_of_threads;

    // create threads and read chunks
    thread *threads = new thread[number_of_threads];
    for (int i = 0; i < number_of_threads; i++)
    {
        threads[i] = thread(
            [&]()
            {
                int file_descriptor = open("test.data", O_RDONLY);
                int offset = i * chunks_per_thread * chunk_size;
                for (int j = 0; j < chunks_per_thread; j++)
                {
                    if (offset >= file_size)
                        break;

                    int read_count = pread(file_descriptor, buffer, chunk_size, offset);
                    if (read_count <= 0)
                        break;
                    offset += read_count;
                }

                // read remaining chunks
                if (i == number_of_threads - 1)
                {
                    for (int j = 0; j < remaining_chunks; j++)
                    {
                        if (offset >= file_size)
                            break;
                        int read_count = pread(file_descriptor, buffer, chunk_size, offset);
                        if (read_count <= 0)
                            break;
                        offset += read_count;
                    }
                }
                close(file_descriptor);
            });
    }

    for (int i = 0; i < number_of_threads; i++)
        threads[i].join();

    delete[] threads;
    delete[] buffer;
    return 0;
}