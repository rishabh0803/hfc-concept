#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

int main()
{
    vector<int> v(100000000, 10);

    cout << v[3] << endl;
    cout << v.size() << endl;

    auto start = chrono::high_resolution_clock::now();
    auto e = v.end();
    for (auto iter = v.begin(); iter != e; ++iter)
    {
        *iter = (*iter) * (*iter);
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << duration.count() << " milliseconds" << endl;
    cout << v[3] << endl;

    return 0;
}