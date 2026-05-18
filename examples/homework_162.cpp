

#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;

void *do_something(void *args)
{
    int k = 0;
    while (1)
    {
        cout << k << " ";
        k++;

        pthread_testcancel(); // Due to this it working on both Windows and Ubuntu
    }

    return NULL;
}

int main()
{
    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, do_something, NULL);
    pthread_create(&t2, NULL, do_something, NULL);
    pthread_create(&t3, NULL, do_something, NULL);

    sleep(2);
    pthread_cancel(t1);
    pthread_cancel(t2);
    pthread_cancel(t3);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}
