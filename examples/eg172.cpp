#include <iostream>
#include <vector>
using namespace std;
int main()
{
    vector<int> v = {10, 20, 30, 40};
    auto iter = v.begin();

    cout << *iter << endl;

    auto k = iter++;

    cout << *k << endl;
    cout << *iter << endl;

    return 0;
}