#include <stdio.h>
#include <stdatomic.h>

int main()
{
    _Atomic int x = 10;

    int y;
    y = atomic_load(&x);

    printf("%d\n", y);
    atomic_store(&x, 55);

    y = atomic_load(&x);
    printf("%d\n", y);

    atomic_fetch_add(&x, 10);
    y = atomic_load(&x);
    printf("%d\n", y);

    atomic_fetch_sub(&x, 5);
    y = atomic_load(&x);
    printf("%d\n", y);

    return 0;
}