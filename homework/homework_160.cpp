#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;

void *do_something(void *args)
{
    bool *w = (bool *)args;
    int k = 0;
    while (*w)
    {
        cout << k << " ";
        k++;
    }

    return NULL;
}

int main()
{
    pthread_t t1, t2, t3;
    volatile bool work = true;

    pthread_create(&t1, NULL, do_something, (void *)&work);
    pthread_create(&t2, NULL, do_something, (void *)&work);
    pthread_create(&t3, NULL, do_something, (void *)&work);

    sleep(2);
    work = false;

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}
