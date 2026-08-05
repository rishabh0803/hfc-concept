#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// Focus of this example: epoll_wait().
//
// In the original server, epoll_wait(..., -1) blocks the loop forever until an
// event arrives, so no other work can be done in the meantime.
//
// Here we pass a timeout of 0 so epoll_wait() returns IMMEDIATELY:
//   - if number_of_events == 0  -> no event yet, go do our "other work"
//   - if number_of_events  > 0  -> an event arrived, handle it
// This way the loop keeps doing background work and still reacts to events.

int main()
{
    int server_file_descriptor;
    int client_file_descriptor;
    int epoll_file_descriptor;
    struct epoll_event ev;
    struct epoll_event events[20];
    int number_of_events;
    int i;
    char buffer[1025];
    unsigned long work_counter = 0; // stands in for our "other work"

    struct sockaddr_in server_sockaddr;
    server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_file_descriptor < 0)
    {
        printf("Unable to setup socket\n");
        return 1;
    }

    bzero(&server_sockaddr, sizeof(server_sockaddr));
    server_sockaddr.sin_port = htons(3500);
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_file_descriptor, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) < 0)
    {
        printf("Unable to bind socket to port 3500\n");
        return 1;
    }

    if (listen(server_file_descriptor, SOMAXCONN) < 0)
    {
        printf("Unable to put socket in listening mode\n");
    }
    epoll_file_descriptor = epoll_create1(0);
    ev.events = EPOLLIN;
    ev.data.fd = server_file_descriptor;
    epoll_ctl(epoll_file_descriptor, EPOLL_CTL_ADD, server_file_descriptor, &ev);

    while (1)
    {
        // timeout 0 => non-blocking poll: return right away instead of waiting.
        number_of_events = epoll_wait(epoll_file_descriptor, events, 20, 0);

        if (number_of_events == 0)
        {
            // No event pending: spend this iteration doing our other work.
            ++work_counter;
            if (work_counter % 5000000 == 0)
            {
                printf("doing other work... (iteration %lu)\n", work_counter);
            }
            continue;
        }

        // At least one event is ready -> handle them.
        for (i = 0; i < number_of_events; ++i)
        {
            if (events[i].data.fd == server_file_descriptor)
            {
                client_file_descriptor = accept(server_file_descriptor, (struct sockaddr *)NULL, NULL);
                printf("Connection accepted\n");
                strcpy(buffer, "Welcome to server");
                write(client_file_descriptor, buffer, strlen(buffer));
                close(client_file_descriptor);
            }
        }
    }

    return 0;
}
