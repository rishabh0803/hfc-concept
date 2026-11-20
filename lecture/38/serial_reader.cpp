#include <iostream>
#include <stdio.h>
#include <chrono>

using namespace std;

int main()
{
    char *buffer;
    buffer = new char[4096];
    FILE *f;
    int read_count;
    char c;
    int i;

    auto start = chrono::high_resolution_clock::now();
    f = fopen("test.data", "rb");
    while (1)
    {
        read_count = fread(buffer, 1, sizeof(buffer), f);
        if (read_count == 0)
            break;
        for (i = 0; i < read_count; ++i)
            c = buffer[i];
    }

    fclose(f);

    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << d.count() << " milliseconds" << endl;
    return 0;
}