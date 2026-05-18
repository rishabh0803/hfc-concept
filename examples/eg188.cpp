#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main()
{
    vector<int> v = {10, 20, 30, 40};

    transform(
        v.begin(),
        v.end(),
        v.begin(),
        [](int value) -> int
        {
            return value + 5;
        });

    for (auto x : v)
        cout << x << endl;

    return 0;
}