#include <iostream>

using namespace std;

struct abcd
{
    int x;
    char m;
    union
    {
        int e;
        double y;
    };
};

int main()
{
    struct abcd k;
    k.x = 10;
    k.m = 'A';
    k.e = 65;
    cout << k.x << endl;
    cout << k.m << endl;
    cout << k.e << endl;
    cout << k.y << endl;
    cout << sizeof(k) << endl;

    return 0;
}