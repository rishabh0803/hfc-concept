#include <vector>
#include <numeric>
#include <iostream>
using namespace std;
int main()
{
    vector<string> v = {"10", "20", "30", "40"};

    string str = accumulate(
        v.begin() + 1, v.end(),
        v[0],
        [](string a, string b)
        {
            return a + string(",") + b;
        });

    cout << str << endl;

    return 0;
}