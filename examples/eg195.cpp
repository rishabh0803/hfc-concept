#include <oneapi / tbb.h>
#include <vector>
#include <iostream>

using namespace std;
using namespace oneapi::tbb;

int main()
{
    vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    parallel_for(
        (size_t)0,
        v.size(),
        [&](int index)
        { v[index] *= v[index]; });

    for (auto x : v)
        cout << x << endl;

    return 0;
}