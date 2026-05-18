#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    int file_descriptor;
    struct sockaddr_in server_address;
    char buffer[1025];
    int n;
    file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (file_descriptor < 0)
    {
        printf("Unable to create socket\n");
        return 1;
    }

    // initialize all the bytes of server_address structure to zero
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(3500); // server port
    if (inet_pton(AF_INET, "172.31.166.138", &server_address.sin_addr) < 0)
    {
        printf("Unable to setup server ip\n");
        return 1;
    }

    if (connect(file_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Unable to connect to server\n");
        return 1;
    }

    n = read(file_descriptor, buffer, 1024);
    if (n == 0)
    {
        printf("Server is not saying anything\n");
    }
    else
    {
        buffer[n] = '\0';
        printf("Server said [%s]\n", buffer);
    }

    return 0;
}
