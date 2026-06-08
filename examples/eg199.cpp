#include <iostream>
#include <string>
#include <chrono>
using namespace std;
int main()
{
    string s = "1231312.2334";
    int x;
    double value;
    auto start = chrono::high_resolution_clock::now();
    while (x < 50000000)
    {
        value = stod(s);
        x++;
    }
    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << d.count() << " milliseconds" << endl;
    return 0;
}