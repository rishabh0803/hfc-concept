#include <iostream>
#include <cstdint>

using namespace std;

union abcd
{
    int x;
    char m;
    double y;
};

int main()
{
    cout << sizeof(union abcd) << endl;
    union abcd a; // 8 bytes will be allocated for a
    a.x = 65;
    cout << a.m << endl;
    cout << a.y << endl;
    a.y = 323.0;
    cout << a.m << "," << (int)a.m << endl;
    cout << "address of x in a " << (uintptr_t)&(a.x) << endl;
    cout << "address of m in a " << (uintptr_t)&(a.m) << endl;
    cout << "address of y in a " << (uintptr_t)&(a.y) << endl;

    return 0;
}