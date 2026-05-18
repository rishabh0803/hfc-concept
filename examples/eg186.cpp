#include <iostream>
#include <vector>
#include <cstdint>
#include <numeric>

using namespace std;

template <typename T>
class binary_operation_plus
{
public:
    T operator()(T p, T q)
    {
        cout << p << "," << q << endl;
        return p + q;
    }
};

int main()
{
    vector<int64_t> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int64_t sum = accumulate(v.begin(), v.end(), 1LL, binary_operation_plus<int64_t>());

    cout << sum << endl;

    return 0;
}