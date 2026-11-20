#include <set>
#include <oneapi/tbb.h>
#include <iostream>

using namespace std;
using namespace tbb;

int main()
{
    set<int> s = {10, 2, 30, 4, 50, 4, 50, 31, 55, 1, 25};
    for (auto x : s)
        cout << x << endl;
    cout << "----------------------------" << endl;

    parallel_for_each(
        s.begin(),
        s.end(),
        [](int x)
        { cout << x << endl; });

    cout << "----------------------------" << endl;

    for (auto x : s)
        cout << x << endl;

    return 0;
}