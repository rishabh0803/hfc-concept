#include <iostream>
#include <cstring>
using namespace std;
#define isdigit(c) (c >= '0' && c <= '9')
int main()
{
    double a;
    int e;
    char c;
    char buffer[1024];
    strcpy(buffer, "1231231.544356,435345353.454565,435534.34534,3e20,345345.454\n");
    char *s;
    s = buffer;
    a = 0.0;
    e = 0;
    while (1)
    {
        while ((c = *s++) != ',' && c != '\n' && isdigit(c))
        {
            a = a * 10.0 + (c - '0');
        }
        if (c == '.')
        {
            while ((c = *s++) != ',' && c != '\n' && isdigit(c))
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
        if (c == ',' || c == '\n')
        {
            printf("%lf\n", a);
            if (c == '\n')
                break;
            a = 0.0;
            e = 0;
        }
    }
    return 0;
}