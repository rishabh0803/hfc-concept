#include <iostream>
#include <linux/io_uring.h>

using namespace std;

int main()
{
    unsigned x; // it is unsigned int
    __u32 y;    // it is unsigned int
    __s32 z;    // it is signed int
    x = 10;
    cout << sizeof(x) << endl;
    return 0;
}