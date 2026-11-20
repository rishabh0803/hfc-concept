#include <iostream>
#include <vector>
#include <cstdint>
#include <numeric>

using namespace std;

int main()
{
    vector<int64_t> v(100000, 10);
    int64_t sum = accumulate(v.begin(), v.end(), 0LL);
    cout << sum << endl;

    return 0;
}