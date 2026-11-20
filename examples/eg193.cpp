#include <vector>
#include <numeric>
#include <iostream>

using namespace std;
// this example is not compiling

int main()
{
    vector<int> v = {10, 20, 30, 40};
    string str = accumulate(
        v.begin() + 1, v.end(),
        to_string(v[0]),
        [](int a, int b)
        {
            return to_string(a) + string(",") + to_string(b);
        });

    cout << str << endl;

    return 0;
}