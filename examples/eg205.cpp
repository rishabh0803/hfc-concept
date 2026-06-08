#include <iostream>
using namespace std;
int main()
{
    int x;
    int num = 0;
    int upper_bound = 31;
    x = 1;
    while (x <= 3)
    {
        cout << num << endl;
        num++;
        if (num == 32)
            num = 0;
        if (num == 0)
            x++;
    }
    return 0;
}