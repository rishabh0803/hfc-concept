#include <iostream>

using namespace std;

#define GOOD 1
#define TALL 2
#define INDIAN 4

int main()
{
    int x;
    x = GOOD | TALL; // oring operation
    /*
    00000001
    00000010
    -----------
    00000011 the value of x
    -----------
    x & TALL
    00000011
    00000010	result will be 2
    x & GOOD
    00000011
    00000001	result will be 1
    x & INDIAN
    00000011
    00000100	result will be zero
    */
    if (x & TALL)
        cout << "Is tall" << endl; // anding operation
    if (x & GOOD)
        cout << "Is good" << endl;
    if (x & INDIAN)
        cout << "Is Indian" << endl;

    return 0;
}