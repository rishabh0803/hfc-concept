#include <iostream>
#include <sys/stat.h>
using namespace std;
int main()
{
    struct stat s;
    if (stat("eg208.out", &s) == 0)
    {
        cout << s.st_size << endl;
    }
    else
    {
        cout << "Unable to fetch size of eg208.out" << endl;
    }
    return 0;
}