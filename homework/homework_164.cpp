#include <pthread.h>
#include <unistd.h>

void *do_something(void *args)
{
    sleep(3);
    return NULL;
}

int main()
{
    pthread_t t1;
    pthread_create(&t1, NULL, do_something, NULL);

    pthread_join(t1, NULL);
    pthread_join(t1, NULL);

    return 0;
}