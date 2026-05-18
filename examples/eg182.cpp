#include <iostream>
using namespace std;
int main()
{
    int x, y, z;
    auto i = [&]()
    {
        x = 10;
        y = 20;
        z = x + y;
    };

    i();

    cout << x << "," << y << "," << z << endl;

    return 0;
}