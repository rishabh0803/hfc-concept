#include <iostream>
#include <stdio.h>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
using namespace std;
int main()
{
    char *buffer;
    // buffer=new char[8*1024];
    buffer = new char[4 * 1024 * 1024];
    int file_descriptor;
    int read_count;
    auto start = chrono::high_resolution_clock::now();
    file_descriptor = open("test.data", O_RDONLY);
    int offset = 0;
    while (1)
    {
        read_count = pread(file_descriptor, buffer, 4 * 1024 * 1024, offset);
        if (read_count <= 0)
            break;
        offset += read_count;
    }
    close(file_descriptor);
    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << d.count() << " milliseconds" << endl;
    delete[] buffer;
    return 0;
}