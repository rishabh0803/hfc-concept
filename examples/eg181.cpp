#include <oneapi/tbb.h>
#include <vector>
#include <chrono>
#include <iostream>

using namespace std;
using namespace oneapi::tbb;

int main()
{
    vector<int64_t> v(10000000, 10);
    auto start = chrono::high_resolution_clock::now();
    int64_t total = parallel_reduce(
        blocked_range<size_t>(0, v.size()),
        0LL,
        [&v](blocked_range<size_t> &range, int64_t acc_sum)
        {
            for (auto i = range.begin(); i != range.end(); ++i) // i is not an iteration type object
            {
                acc_sum += v[i];
            }
            return acc_sum;
        },
        [](int p, int q) -> int
        {
            return p + q;
        });

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << duration.count() << " milliseconds" << endl;
    cout << "Total " << total << endl;

    return 0;
}