#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

int main()
{
    cout << O_RDONLY << endl;
    cout << O_RDWR << endl;
    cout << O_CREAT << endl;
    cout << O_TRUNC << endl;
    cout << (O_RDWR | O_CREAT | O_TRUNC) << endl;

    return 0;
}