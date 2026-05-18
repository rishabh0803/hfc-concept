#include <utils.h>
#include <vector>
#include <iostream>

using namespace std;

int main()
{
    vector<uint32_t> v;

    fill_random(v, 10, 20, 50);

    for (auto x : v)
        cout << x << endl;

    return 0;
}