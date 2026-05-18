#include <iostream>
#include <vector>
#include <utils.h>
#include <chrono>
#include <numeric>
#include <execution>

using namespace std;

int main()
{
    vector<uint32_t> v;
    fill_random(v, 100000000, 10, 100);
    uint64_t sum = 0;

    auto start = chrono::high_resolution_clock::now();
    sum = reduce(execution::par, v.begin(), v.end());

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Sum : " << sum << endl;
    cout << duration.count() << " milliseconds" << endl;

    return 0;
}