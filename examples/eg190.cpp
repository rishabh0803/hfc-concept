#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

int main()
{
    vector<int> v = {10, 20, 30, 40, 50, 60};
    vector<string> vs(v.size());

    transform(
        v.begin(), v.end(),
        vs.begin(),
        [](int num)
        {
            return string("Number : ") + to_string(num);
        });

    for (auto str : vs)
        cout << str << endl;

    return 0;
}