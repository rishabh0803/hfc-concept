// Code to compare time taken to copy a file using parallelism with
// 1. pread and pwrite
// 2. mmap
// 3. preadv and pwritev

#include <iostream>
#include <stdio.h>
#include <chrono>

#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#include <sys/uio.h>

#include <thread>

using namespace std;
using namespace std::chrono;

void copy_using_pread_pwrite_parallel()
{
    constexpr uint64_t CHUNK_SIZE = 4ULL * 1024 * 1024; // 4 MB

    int src_fd = open("test.data", O_RDONLY);
    if (src_fd < 0)
    {
        perror("open source");
        return;
    }

    int dst_fd = open("test_copy_5.data", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dst_fd < 0)
    {
        perror("open destination");
        close(src_fd);
        return;
    }

    struct stat st;
    if (fstat(src_fd, &st) != 0)
    {
        perror("fstat");
        close(src_fd);
        close(dst_fd);
        return;
    }

    uint64_t file_size = st.st_size;

    // Preallocate destination file
    int rc = ftruncate(dst_fd, file_size);
    if (rc != 0)
    {
        perror("ftruncate");
        close(src_fd);
        close(dst_fd);
        return;
    }

    uint64_t hw_threads = thread::hardware_concurrency();
    int number_of_threads = hw_threads;

    std::cout << "File size      : " << file_size << " bytes\n";
    std::cout << "Chunk size     : " << CHUNK_SIZE << " bytes\n";
    std::cout << "Thread count   : " << number_of_threads << "\n";

    auto start = high_resolution_clock::now();

    thread workers[number_of_threads];

    for (int i = 0; i < number_of_threads; i++)
    {
        uint64_t start_offset = (file_size * i) / number_of_threads;
        uint64_t end_offset = (file_size * (i + 1)) / number_of_threads;

        workers[i] = thread(
            [=]()
            {
                char buffer[CHUNK_SIZE];

                uint64_t offset = start_offset;

                while (offset < end_offset)
                {
                    uint64_t bytes_to_read = min(CHUNK_SIZE, end_offset - offset);
                    ssize_t bytes_read = pread(src_fd, buffer, bytes_to_read, offset);

                    if (bytes_read <= 0)
                        break;

                    ssize_t bytes_written = pwrite(dst_fd, buffer, bytes_read, offset);
                    offset += bytes_read;
                }
            });
    }

    for (auto &t : workers)
        t.join();

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    std::cout << "Time taken to copy using pread and pwrite parallel: " << duration.count() << " ms" << endl;

    close(src_fd);
    close(dst_fd);
}

void copy_using_mmap_parallel()
{
    int src_fd = open("test.data", O_RDONLY);
    if (src_fd < 0)
    {
        perror("open source");
        return;
    }

    int dst_fd = open("test_copy_6.data", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (dst_fd < 0)
    {
        perror("open destination");
        close(src_fd);
        return;
    }

    struct stat st;
    if (fstat(src_fd, &st) != 0)
    {
        perror("fstat");
        close(src_fd);
        close(dst_fd);
        return;
    }

    uint64_t file_size = st.st_size;

    // Preallocate destination file
    int rc = ftruncate(dst_fd, file_size);
    if (rc != 0)
    {
        perror("ftruncate");
        close(src_fd);
        close(dst_fd);
        return;
    }

    uint64_t hw_threads = thread::hardware_concurrency();
    int number_of_threads = hw_threads;

    std::cout << "File size      : " << file_size << " bytes\n";
    std::cout << "Thread count   : " << number_of_threads << "\n";

    auto start = high_resolution_clock::now();

    char *src_map = (char *)mmap(NULL, file_size, PROT_READ, MAP_SHARED, src_fd, 0);
    char *dst_map = (char *)mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, dst_fd, 0);

    thread workers[number_of_threads];

    for (int i = 0; i < number_of_threads; i++)
    {
        uint64_t start_offset = (file_size * i) / number_of_threads;
        uint64_t end_offset = (file_size * (i + 1)) / number_of_threads;

        workers[i] = thread(
            [=]()
            {
                memcpy(dst_map + start_offset, src_map + start_offset, end_offset - start_offset);
            });
    }

    for (auto &t : workers)
        t.join();

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    std::cout << "Time taken to copy using mmap parallel: " << duration.count() << " ms" << endl;

    munmap(src_map, file_size);
    munmap(dst_map, file_size);

    close(src_fd);
    close(dst_fd);
}

void copy_using_preadv_pwritev_parallel()
{
    constexpr uint64_t CHUNK_SIZE = 4ULL * 1024 * 1024; // 4 MB

    int src_fd = open("test.data", O_RDONLY);
    if (src_fd < 0)
    {
        perror("open source");
        return;
    }

    int dst_fd = open("test_copy_7.data", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dst_fd < 0)
    {
        perror("open destination");
        close(src_fd);
        return;
    }

    struct stat st;
    if (fstat(src_fd, &st) != 0)
    {
        perror("fstat");
        close(src_fd);
        close(dst_fd);
        return;
    }

    uint64_t file_size = st.st_size;

    // Preallocate destination file
    int rc = ftruncate(dst_fd, file_size);

    if (rc != 0)
    {
        perror("ftruncate");
        close(src_fd);
        close(dst_fd);
        return;
    }

    uint64_t hw_threads = thread::hardware_concurrency();
    int number_of_threads = hw_threads;

    std::cout << "File size      : " << file_size << " bytes\n";
    std::cout << "Chunk size     : " << CHUNK_SIZE << " bytes\n";
    std::cout << "Thread count   : " << number_of_threads << "\n";

    auto start = high_resolution_clock::now();

    thread workers[number_of_threads];
    for (int i = 0; i < number_of_threads; i++)
    {
        uint64_t start_offset = (file_size * i) / number_of_threads;
        uint64_t end_offset = (file_size * (i + 1)) / number_of_threads;
        workers[i] = thread(
            [=]()
            {
                uint64_t offset = start_offset;
                while (offset < end_offset)
                {
                    uint64_t chunk_size = std::min(CHUNK_SIZE, end_offset - offset);
                    struct iovec iov[1];
                    iov[0].iov_base = (void *)(uintptr_t)offset;
                    iov[0].iov_len = chunk_size;
                    preadv(src_fd, iov, 1, offset);
                    pwritev(dst_fd, iov, 1, offset);
                    offset += chunk_size;
                }
            });
    }

    for (auto &t : workers)
        t.join();

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "Time taken to copy using preadv and pwritev parallel: " << duration.count() << " ms" << endl;

    close(src_fd);
    close(dst_fd);
}

int main()
{
    copy_using_pread_pwrite_parallel();

    copy_using_mmap_parallel();

    copy_using_preadv_pwritev_parallel();

    return 0;
}