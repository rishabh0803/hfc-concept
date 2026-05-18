#include <oneapi/tbb.h>
#include <vector>
#include <chrono>
#include <iostream>
#include <unordered_set>

using namespace std;
using namespace oneapi::tbb;

int main()
{
    unordered_set<thread::id> thread_ids;
    vector<int64_t> v(10000000, 10);

    auto start = chrono::high_resolution_clock::now();

    int64_t total = parallel_reduce(
        blocked_range<size_t>(0, v.size()),
        0LL,
        [&](blocked_range<size_t> &range, int64_t acc_sum)
        {
            thread_ids.insert(this_thread::get_id());
            auto start_at = range.begin();
            auto end_at = range.end();
            cout << "s:" << start_at << ",e:" << end_at << endl;
            for (auto i = start_at; i != end_at; ++i)
            {
                acc_sum += v[i];
            }
            return acc_sum;
        },
        [](int p, int q) -> int
        { return p + q; }
    );

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << duration.count() << " milliseconds" << endl;
    cout << "Total " << total << endl;
    cout << "Number of threads created : " << thread_ids.size() << endl;

    return 0;
}