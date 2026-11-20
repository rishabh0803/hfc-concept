#include <iostream>
#include <vector>
#include <cstdint>
#include <numeric>

using namespace std;

int main()
{
    vector<int64_t> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int64_t sum = accumulate(
        v.begin(), v.end(),
        1LL,
        [](int64_t p, int64_t q) -> int64_t
        {
            cout << p << "," << q << endl;
            return p * q;
        });

    cout << sum << endl;

    return 0;
}