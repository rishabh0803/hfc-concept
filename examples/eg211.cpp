#include <iostream>
#include <string>
#include <chrono>
#include <cstring>
#include <stdlib.h>
using namespace std;
#define isdigit(c) (c >= '0' && c <= '9')
int main()
{
    double a;
    int e;
    int c;
    char buffer[1024];
    strcpy(buffer, "1231312.2334");
    int x;
    int index;
    double value;
    auto start = chrono::high_resolution_clock::now();
    x = 1;
    while (x < 50)
    {
        a = 0.0;
        e = 0;
        index = 0;
        while ((c = buffer[index++]) != '\0' && isdigit(c))
        {
            a = a * 10.0 + (c - '0');
        }
        if (c == '.')
        {
            while ((c = buffer[index++]) != '\0' && isdigit(c))
            {
                a = a * 10.0 + (c - '0');
                e = e - 1;
            }
        }
        if (c == 'e' || c == 'E')
        {
            int sign = 1;
            int i = 0;
            c = buffer[index++];
            if (c == '+')
                c = buffer[index++];
            else if (c == '-')
            {
                c = buffer[index++];
                sign = -1;
            }
            while (isdigit(c))
            {
                i = i * 10 + (c - '0');
                c = buffer[index++];
            }
            e += i * sign;
        }
        while (e > 0)
        {
            a *= 10.0;
            e--;
        }
        while (e < 0)
        {
            a *= 0.1;
            e++;
        }
        cout << x << ")" << a << endl;
        x++;
    }
    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << d.count() << " milliseconds" << endl;
    return 0;
}