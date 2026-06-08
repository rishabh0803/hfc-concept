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
    char *s;
    strcpy(buffer, "1231312.2334");
    int x;
    double value;
    auto start = chrono::high_resolution_clock::now();
    while (x < 50000000)
    {
        a = 0.0;
        e = 0;
        s = buffer;
        while ((c = *s++) != '\0' && isdigit(c))
        {
            a = a * 10.0 + (c - '0');
        }
        if (c == '.')
        {
            while ((c = *s++) != '\0' && isdigit(c))
            {
                a = a * 10.0 + (c - '0');
                e = e - 1;
            }
        }
        if (c == 'e' || c == 'E')
        {
            int sign = 1;
            int i = 0;
            c = *s++;
            if (c == '+')
                c = *s++;
            else if (c == '-')
            {
                c = *s++;
                sign = -1;
            }
            while (isdigit(c))
            {
                i = i * 10 + (c - '0');
                c = *s++;
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
        x++;
    }
    auto end = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << d.count() << " milliseconds" << endl;
    return 0;
}