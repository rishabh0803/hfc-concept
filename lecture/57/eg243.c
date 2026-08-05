#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <poll.h>

// In this example we are not writing against or reading from whatever memfd_create created
// This we will handle in next session

struct wrapper
{
    int memfd;
    int fds[2];
};

void *simulator(void *v)
{
    int i;
    const char *str = "God is great, Ujjain is the city of GODS#";

    struct wrapper *ptr;
    ptr = (struct wrapper *)v;

    for (i = 0; str[i] != '\0'; ++i)
    {
        printf("[%c]\n", str[i]);
        write(ptr->fds[1], &str[i], 1);
        sleep(1);
    }

    close(ptr->fds[1]);
}

int main()
{
    int x;
    struct wrapper w;
    struct pollfd pfd;
    char m;
    pthread_t t;

    w.memfd = memfd_create("my_custom_ram_file", MFD_CLOEXEC);
    if (w.memfd < 0)
    {
        printf("Unable to create file in memory\n");
        return 1;
    }

    if (pipe(w.fds) < 0)
    {
        close(w.memfd);
        printf("Unable to create channels(pipe)\n");
        return 1;
    }

    t = pthread_create(&t, NULL, simulator, &w);

    // poll for reading starts here
    pfd.fd = w.fds[0];
    pfd.events = POLLIN;
    while (1)
    {
        x = poll(&pfd, 1, 0);
        // because of 0, returns immediately

        if (x > 0 && (pfd.revents & POLLIN))
        {
            read(w.fds[0], &m, 1);
            printf("(%c)\n", m);
            if (m == '#')
                break;
        }
        else
        {
            // printf(".");
        }
    }

    close(w.fds[0]);
    pthread_join(t, NULL);
    close(w.memfd);

    return 0;
}