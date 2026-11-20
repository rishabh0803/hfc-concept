#include <oneapi/tbb.h>
#include <iostream>

using namespace std;
using namespace oneapi::tbb;

void sam()
{
    for (int e = 0; e <= 10; e++)
        cout << e << " ";
}

int main()
{
    parallel_invoke(
        []()
        {
            for (int j = 403; j <= 530; ++j)
                cout << j << " ";
        },
        sam,
        []()
        {
            for (int k = 703; k <= 870; ++k)
                cout << k << " ";
        });

    return 0;
}