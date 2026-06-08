#include <iostream>
#include <float.h>
using namespace std;
int main()
{
    double a = DBL_MIN;
    double b = DBL_MAX;
    double c = -DBL_MAX; // if you want to go for minimum possible value of double, use -DBL_MAX
    cout << a << endl;
    cout << b << endl;
    cout << c << endl;
    cout << (a < c) << endl;
    cout << (a == c) << endl;
    cout << (c < a) << endl;
    return 0;
}