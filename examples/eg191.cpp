#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

int main()
{
    vector<int> v1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    vector<int> v2 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    vector<int> v3(v1.size());

    transform(
        v1.begin(), v1.end(),
        v2.begin(),
        v3.begin(),
        [](int x, int y)
        {
            return x + y;
        });

    for (auto x : v3)
        cout << x << endl;

    return 0;
}