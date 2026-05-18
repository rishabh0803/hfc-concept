#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main()
{
    int server_file_descriptor;
    int client_file_descriptor;
    char buffer[1025];
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

    printf("INADDR_ANY %d\n", INADDR_ANY);
    printf("SOMAXCONN %d\n", SOMAXCONN);
    if (listen(server_file_descriptor, SOMAXCONN) < 0)
    {
        printf("Unable to put socket in listening mode\n");
    }

    while (1)
    {
        printf("Waiting to accept connection......\n");
        client_file_descriptor = accept(server_file_descriptor, (struct sockaddr *)NULL, NULL);
        printf("Connection accepted\n"); // 2nd argument at later stage will be address of structure created from struct sockaddr

        // 3rd argument will be address of int // ideally the following code should go on separate thread
        strcpy(buffer, "Welcome to server");
        write(client_file_descriptor, buffer, strlen(buffer));
        close(client_file_descriptor);
    }

    return 0;
}
