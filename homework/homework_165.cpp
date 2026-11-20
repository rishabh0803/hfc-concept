#include <iostream>
#include <vector>
#include <ctime>
#include <cstdint>
#include <chrono>
#include <numeric>
#include <execution>

using namespace std;

int main()
{
    int size = 1000000000;
    uint32_t min = 0;
    uint32_t max = RAND_MAX;

    vector<uint32_t> v(size);
    srand(time(NULL));

    for (uint32_t i = 0; i < size; i++)
        v[i] = rand() % (max - min) + min;

    // start
    auto start = chrono::high_resolution_clock::now();

    uint64_t sum = reduce(execution::par, v.begin(), v.end(), 0);

    auto end = chrono::high_resolution_clock::now();
    auto diff = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << diff.count() << endl;

    start = chrono::high_resolution_clock::now();

    sum = 0;
    for (auto x : v)
        sum += x;

    end = chrono::high_resolution_clock::now();
    diff = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << diff.count() << endl;

    return 0;
}