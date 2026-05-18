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
        printf("Connection accepted\n");

        // recieve file content from client and create file with same name and content
        get_file_content(client_file_descriptor);
        close(client_file_descriptor);
    }

    return 0;
}

void get_file_content(int file_descriptor)
{
    char buffer[8 * 1024 + 1];
    int bytes_read;

    char file_name[256];
    long int file_content_size;

    // Recieve file name and content size from client
    bytes_read = recv(file_descriptor, buffer, sizeof(buffer), 0);
    if (bytes_read < 0)
    {
        printf("Unable to recieve file information\n");
        return;
    }

    // iterate buffer to get file name and content size
    buffer[bytes_read] = '\0';
    for (int i = 0; i < bytes_read; i++)
    {
        if (buffer[i] == ':')
        {
            buffer[i] = '\0';
            strcpy(file_name, buffer);
            file_content_size = atol(&buffer[i + 1]);
            break;
        }
    }

    printf("File name: %s, File content size: %ld\n", file_name, file_content_size);

    FILE *file = fopen(file_name, "wb");
    if (file == NULL)
    {
        printf("Unable to create file\n");
        return;
    }

    long int total_bytes_received = 0;
    while (total_bytes_received < file_content_size)
    {
        bytes_read = recv(file_descriptor, buffer, sizeof(buffer), 0);
        if (bytes_read < 0)
        {
            printf("Unable to recieve file content\n");
            fclose(file);
            return;
        }
        fwrite(buffer, sizeof(char), bytes_read, file);
        total_bytes_received += bytes_read;
    }

    printf("File recieved successfully\n");
    fclose(file);
}
