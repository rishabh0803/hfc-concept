#include <stdio.h>
#include <string.h>

int main()
{
    char buffer[1024];
    FILE *f;
    int x;
    memset(buffer, 'A', sizeof(buffer));
    f = fopen("test.data", "wb");
    for (x = 1; x < 5 * 1024 * 1024; ++x)
    {
        fwrite(buffer, sizeof(buffer), 1, f);
    }
    fclose(f);
    return 0;
}