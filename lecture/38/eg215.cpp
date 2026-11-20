#include <iostream>
#include <unistd.h>

using namespace std;

int main()
{
    size_t page_size = sysconf(_SC_PAGESIZE);
    cout << page_size << endl;
    return 0;
}