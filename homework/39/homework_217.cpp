// Code to compare time taken to copy a file using without parallelism
// 1. fread and fwrite
// 2. read and write
// 3. pread and pwrite
// 4. mmap

#include <iostream>
#include <stdio.h>
#include <chrono>

#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

using namespace std;
using namespace std::chrono;

void copy_using_fread_fwrite()
{
    char *buffer;
    buffer = new char[8 * 1024];

    FILE *f1, *f2;
    int read_count;

    auto start = chrono::high_resolution_clock::now();
    f1 = fopen("test.data", "rb");
    f2 = fopen("test_copy_1.data", "wb");

    while (1)
    {
        // print the progress of copying every 40 MB
        static uint64_t total_bytes_copied = 0;
        if (total_bytes_copied % (40ULL * 1024 * 1024) == 0)
        {
            cout << "Copied " << total_bytes_copied / (1024 * 1024) << " MB\n";
        }

        read_count = fread(buffer, 1, sizeof(buffer), f1);
        if (read_count == 0)
            break;

        fwrite(buffer, 1, read_count, f2);
        total_bytes_copied += read_count;
    }

    fclose(f1);
    fclose(f2);

    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Time taken to copy using fread and fwrite : " << d.count() << " milliseconds" << endl;

    delete[] buffer;
    return;
}

void copy_using_read_write()
{
    char *buffer;
    buffer = new char[8 * 1024];

    int file_descriptor1, file_descriptor2;
    int read_count;

    auto start = chrono::high_resolution_clock::now();
    file_descriptor1 = open("test.data", O_RDONLY);
    file_descriptor2 = open("test_copy_2.data", O_WRONLY | O_CREAT | O_TRUNC, 0666);

    while (1)
    {
        // print the progress of copying every 40 MB
        static uint64_t total_bytes_copied = 0;
        if (total_bytes_copied % (40ULL * 1024 * 1024) == 0)
        {
            cout << "Copied " << total_bytes_copied / (1024 * 1024) << " MB\n";
        }

        read_count = read(file_descriptor1, buffer, sizeof(buffer));
        if (read_count <= 0)
            break;

        write(file_descriptor2, buffer, read_count);
        total_bytes_copied += read_count;
    }
    close(file_descriptor1);
    close(file_descriptor2);

    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Time taken to copy using read and write : " << d.count() << " milliseconds" << endl;

    delete[] buffer;
    return;
}

void copy_using_pread_pwrite()
{
    char *buffer;
    buffer = new char[8 * 1024];

    int file_descriptor1, file_descriptor2;
    int read_count;

    auto start = chrono::high_resolution_clock::now();
    file_descriptor1 = open("test.data", O_RDONLY);
    file_descriptor2 = open("test_copy_3.data", O_WRONLY | O_CREAT | O_TRUNC, 0666);

    int offset = 0;
    static uint64_t total_bytes_copied = 0;
    while (1)
    {
        read_count = pread(file_descriptor1, buffer, sizeof(buffer), offset);
        if (read_count <= 0)
            break;

        pwrite(file_descriptor2, buffer, read_count, offset);
        offset += read_count;
        total_bytes_copied += read_count;
        if (total_bytes_copied % (40ULL * 1024 * 1024) == 0)
        {
            cout << "Copied " << total_bytes_copied / (1024 * 1024) << " MB\n";
        }
    }
    close(file_descriptor1);
    close(file_descriptor2);

    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Time taken to copy using pread and pwrite : " << d.count() << " milliseconds" << endl;

    delete[] buffer;
    return;
}

void copy_using_mmap()
{
    string source = "test.data";
    string target = "test_copy_4.data";

    struct stat s;
    if (stat(source.c_str(), &s) < 0)
    {
        cout << "Unable to determine size of " << source << endl;
        return;
    }

    size_t source_file_size = s.st_size;
    int source_fd;
    source_fd = open(source.c_str(), O_RDONLY);
    if (source_fd < 0)
    {
        cout << "Unable to open " << source << endl;
        return;
    }

    char *source_ptr = (char *)mmap(NULL, source_file_size, PROT_READ, MAP_PRIVATE, source_fd, 0);
    if (source_ptr == MAP_FAILED)
    {
        close(source_fd);
        cout << "Unable to map source file" << endl;
        return;
    }

    int target_fd;
    target_fd = open(target.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (target_fd < 0)
    {
        munmap(source_ptr, source_file_size);
        close(source_fd);
        cout << "Unable to open " << target << endl;
        return;
    }

    if (ftruncate(target_fd, source_file_size) < 0)
    {
        cout << "Unable to setup size of " << target << endl;
        munmap(source_ptr, source_file_size);
        close(source_fd);
        close(target_fd);
        return;
    }

    char *target_ptr = (char *)mmap(NULL, source_file_size, PROT_READ | PROT_WRITE, MAP_SHARED, target_fd, 0);
    if (target_ptr == MAP_FAILED)
    {
        cout << "Unable to map " << target << endl;
        munmap(source_ptr, source_file_size);
        close(source_fd);
        close(target_fd);
        return;
    }

    auto start = chrono::high_resolution_clock::now();

    memcpy(target_ptr, source_ptr, source_file_size);

    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Time taken to copy using mmap : " << d.count() << " milliseconds" << endl;

    munmap(source_ptr, source_file_size);
    munmap(target_ptr, source_file_size);
    close(source_fd);
    close(target_fd);

    return;
}

int main()
{
    copy_using_fread_fwrite();

    copy_using_read_write();

    copy_using_pread_pwrite();

    copy_using_mmap();

    return 0;
}