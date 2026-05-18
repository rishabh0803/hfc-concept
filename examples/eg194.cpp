#include <oneapi/tbb.h>
#include <vector>
#include <iostream>

using namespace std;
using namespace oneapi::tbb;

int main()
{
    vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    parallel_for(
        blocked_range<size_t>(0, v.size()),
        [&](blocked_range<size_t> &range)
        {
            for (auto i = range.begin(); i != range.end(); ++i)
                v[i] *= v[i];
        });

    for (auto x : v)
        cout << x << endl;

    return 0;
}