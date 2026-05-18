#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    struct stat s;
    long int file_content_size;
    if (stat(argv[1], &s) == 0)
    {
        printf("File size: %ld bytes\n", s.st_size);
        file_content_size = s.st_size;
    }
    else
    {
        printf("Unable to get file information\n");
        return 1;
    }

    int file_descriptor;
    struct sockaddr_in server_address;

    file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (file_descriptor < 0)
    {
        printf("Unable to create socket\n");
        return 1;
    }

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(3500);

    if (inet_pton(AF_INET, "172.31.166.138", &server_address.sin_addr) < 0)
    {
        printf("Unable to setup server\n");
        return 1;
    }

    if (connect(file_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Unable to connect to server\n");
        return 1;
    }

    // Connected to server, now send file content
    upload_file_chunk(file_descriptor, argv[1], file_content_size);
    close(file_descriptor);

    return 0;
}

void upload_file_chunk(int file_descriptor, char *file_name, long int file_content_size)
{
    FILE *file_pointer;
    char buffer[8 * 1024];
    size_t bytes_read;

    file_pointer = fopen(file_name, "rb");
    if (file_pointer == NULL)
    {
        printf("Unable to open file\n");
        return;
    }

    sprintf(buffer, "%s:%ld", file_name, file_content_size);

    if (send(file_descriptor, buffer, strlen(buffer), 0) < 0)
    {
        printf("Send failed\n");
        return;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file_pointer)) > 0)
    {
        if (send(file_descriptor, buffer, bytes_read, 0) < 0)
        {
            printf("Send failed\n");
            return;
        }
    }

    fclose(file_pointer);
}
