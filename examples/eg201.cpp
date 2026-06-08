#include <iostream>
#include <string>
#include <chrono>
#include <stdlib.h>
using namespace std;
int main()
{
    const char *s = "1231312.2334";
    int x;
    double value;
    auto start = chrono::high_resolution_clock::now();
    while (x < 50000000)
    {
        value = atof(s);
        x++;
    }
    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << d.count() << " milliseconds" << endl;
    return 0;
}