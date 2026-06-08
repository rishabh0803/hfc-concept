#include <iostream>
#include <stdio.h>
#include <chrono>
using namespace std;
int main()
{
    char *buffer;
    buffer = new char[8 * 1024];
    FILE *f;
    int read_count;
    auto start = chrono::high_resolution_clock::now();
    f = fopen("test.data", "rb");
    while (1)
    {
        read_count = fread(buffer, 1, sizeof(buffer), f);
        if (read_count == 0)
            break;
    }
    fclose(f);
    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << d.count() << " milliseconds" << endl;
    return 0;
}