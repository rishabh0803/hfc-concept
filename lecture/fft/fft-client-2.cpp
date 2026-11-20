#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstdint>
#include <vector>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define HANDSHAKE_PACKET 'H'
#define FILE_DATA_PACKET 'D'

using namespace std;

uint64_t memory_requirement = 0;
vector<pair<uint64_t, uint64_t>> files;
vector<pair<uint32_t, uint32_t>> work_vector;
uint64_t total_bytes_to_upload;
uint64_t max_path_len = 0;
uint64_t entry_count = 0;
char *entry_buffer;
char *file_paths;
int len;
int _index;
int server_socket_descriptor;

enum _operation_enum
{
    COPY_FILE,
    CREATE_DIRECTORY_AND_COPY_FILE,
    MKDIR
};

typedef struct _handshake_request_packet
{
    char packet_type;
    char buffer[7922];
    char filename[256];
    char overwrite;
    int operation;
    uint64_t source_file_size;
} handshake_request_packet;

typedef struct _handshake_response_packet
{
    char operation_status;
    char buffer[8191];
} handshake_response_packet;

typedef struct _request_packet
{
    char packet_type;
    char buffer[8187];
    uint32_t chunk_number;
} request_packet;

typedef struct _response_packet
{
    char buffer[8192];
} response_packet;

enum fft_enums
{
    SUCCESS,
    SOCKET_ERROR,
    SERVER_IP_ERROR,
    CONNECT_ERROR,
    NOT_CONNECTED_ERROR,
    IS_FILE,
    IS_DIRECTORY,
    INVALID_SOURCE_PATH,
    SOURCE_OPEN_ERROR,
    CANNOT_REMOTE_COPY_ERROR,
    CANNOT_CREATE_REMOTE_DIRECTORY_ERROR
};

char error_message[4096];
void setup_globals(char *);
int fft_connect(const char *ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        return SOCKET_ERROR;
    }
    // initialize all the bytes of server_address structure to zero
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    int ret = inet_pton(AF_INET, ip, &server_address.sin_addr);
    if (ret == 0)
    {
        close(fd);
        return SERVER_IP_ERROR;
    }
    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        close(fd);
        return CONNECT_ERROR;
    }
    server_socket_descriptor = fd;
    return SUCCESS;
}

void write_error(int error)
{
    switch (error)
    {
    case SOCKET_ERROR:
        cout << "Socket error" << endl;
        break;
    case SERVER_IP_ERROR:
        cout << "server ip error" << endl;
        break;
    case CONNECT_ERROR:
        cout << "Unable to connect to server" << endl;
        break;
    case INVALID_SOURCE_PATH:
        cout << "source path does not exist" << endl;
        break;
    case NOT_CONNECTED_ERROR:
        cout << "Not connected to server" << endl;
        break;
    }
}

void fft_close_connection()
{
    if (server_socket_descriptor != -1)
        close(server_socket_descriptor);
    server_socket_descriptor = -1;
}

int is_connected()
{
    return server_socket_descriptor != -1;
}

int get_file_type(char *path)
{
    struct stat s;
    if (stat(path, &s) < 0)
        return 0;
    if (S_ISREG(s.st_mode))
        return IS_FILE;
    if (S_ISDIR(s.st_mode))
        return IS_DIRECTORY;
    return INVALID_SOURCE_PATH;
}

int copy_file_to_remote(char *source, char *target, char overwrite)
{
    size_t bytes_fetched;
    struct stat s;
    stat(source, &s);
    handshake_request_packet handshake_request_pkt;
    handshake_request_pkt.source_file_size = s.st_size;
    handshake_request_pkt.operation = COPY_FILE;
    strcpy(handshake_request_pkt.filename, basename(source));
    handshake_request_pkt.overwrite = overwrite;
    int fd;
    fd = open(source, O_RDONLY | O_BINARY);
    if (fd < 0)
        return SOURCE_OPEN_ERROR;
    handshake_request_pkt.packet_type = HANDSHAKE_PACKET;
    strcpy(handshake_request_pkt.buffer, target);
    send(server_socket_descriptor, &handshake_request_pkt, sizeof(handshake_request_pkt), 0);
    // what to do if send fails
    handshake_response_packet handshake_response_pkt;
    size_t bytes_remaining = sizeof(handshake_response_pkt);
    char *ptr;
    ptr = (char *)&handshake_response_pkt;
    while (bytes_remaining > 0)
    {
        bytes_fetched = read(server_socket_descriptor, ptr, bytes_remaining);
        bytes_remaining -= bytes_fetched;
        ptr += bytes_fetched;
    }
    if (!(handshake_response_pkt.operation_status))
    {
        close(fd);
        strcpy(error_message, handshake_response_pkt.buffer);
        printf("%s\n", error_message); // to be removed later on
        return CANNOT_REMOTE_COPY_ERROR;
    }
    uint64_t bytes_fetched_from_file = 0;
    uint64_t bytes_yet_to_be_fetched = handshake_request_pkt.source_file_size;
    request_packet pkt;
    pkt.chunk_number = 1;
    pkt.packet_type = FILE_DATA_PACKET;
    uint32_t bytes_to_read;
    uint32_t offset;
    while (bytes_yet_to_be_fetched > 0)
    {
        bytes_to_read = 8187;
        if (bytes_to_read > bytes_yet_to_be_fetched)
            bytes_to_read = bytes_yet_to_be_fetched;
        offset = 0;
        while (bytes_to_read > 0)
        {
            bytes_fetched_from_file = read(fd, pkt.buffer + offset, bytes_to_read);
            offset += bytes_fetched_from_file;
            bytes_to_read -= bytes_fetched_from_file;
        }
        bytes_yet_to_be_fetched -= offset;
        send(server_socket_descriptor, &pkt, sizeof(pkt), 0);
        pkt.chunk_number++;
        cout << "waiting for 30 secs" << endl;
        sleep(30);
    }
    close(fd);
    response_packet res_pkt;
    int n = read(server_socket_descriptor, &res_pkt, sizeof(response_packet)); // has to go in a loop
    if (n == 0)
        cout << "Did not get anything in the end" << endl;
    else
        cout << res_pkt.buffer[0] << " received back in the end" << endl;
    return SUCCESS;
}

int copy_directory_to_remote(char *source, char *target, char overwrite)
{
    // source has been verified and is a directory
    // we need to tell the server to create target directory structure
    size_t bytes_fetched;
    struct stat s;
    stat(source, &s);
    handshake_request_packet handshake_request_pkt;
    handshake_request_pkt.source_file_size = 0;
    handshake_request_pkt.operation = MKDIR;
    handshake_request_pkt.filename[0] = '\0';
    handshake_request_pkt.overwrite = 'Y';
    handshake_request_pkt.packet_type = HANDSHAKE_PACKET;
    strcpy(handshake_request_pkt.buffer, target);
    send(server_socket_descriptor, &handshake_request_pkt, sizeof(handshake_request_pkt), 0);

    // what to do if send fails
    handshake_response_packet handshake_response_pkt;
    size_t bytes_remaining = sizeof(handshake_response_pkt);
    char *ptr;
    ptr = (char *)&handshake_response_pkt;
    while (bytes_remaining > 0)
    {
        bytes_fetched = read(server_socket_descriptor, ptr, bytes_remaining);
        bytes_remaining -= bytes_fetched;
        ptr += bytes_fetched;
    }
    if (!(handshake_response_pkt.operation_status))
    {
        printf("Unable to create directory %s on remote end\n", target);
        return CANNOT_CREATE_REMOTE_DIRECTORY_ERROR;
    }
    // if server's response is positive
    int len = strlen(source);
    char *path = (char *)malloc(sizeof(char) * (len + 2));
    strcpy(path, source);
    if (path[len - 1] != '/')
    {
        path[len] = '/';
        path[len + 1] = '\0';
        len++;
    }
    setup_globals(path);
    // iterate number of entries and upload or mkdir
    /*
    scenario 1
    pqrst and its contents should copied to /lmnop
    */
    free(file_paths);
    free(path);
    return SUCCESS;
}

int copy_to_remote(char *source, char *target, char overwrite)
{
    int error;
    if (!is_connected)
        return NOT_CONNECTED_ERROR;
    int file_type;
    file_type = get_file_type(source);
    if (file_type == INVALID_SOURCE_PATH)
        return INVALID_SOURCE_PATH;
    if (file_type == IS_FILE)
        return copy_file_to_remote(source, target, overwrite);
    else if (file_type == IS_DIRECTORY)
        return copy_directory_to_remote(source, target, overwrite);
    return SUCCESS; // this is unreachable
}

void populate_work_vector()
{
    // count available cores
    // entry_cout<(2*cores-1) setup work_vector ===== files | story ends
    // number_of_threads=2*cores-1
    // bytes per thread = total_bytes_to_upload/number_of_threads
    // setup work vector, implement distribution | accumulation logic
}

void compute_memory_requirement(const char *path)
{
    int total;
    struct stat s;
    char *entry_path;
    DIR *dir;
    dir = opendir(path);
    if (dir == NULL)
        return;
    struct dirent *entry;
    int path_len;
    int entry_name_len;
    path_len = strlen(path);
    bool had_entries = false;
    while (entry = readdir(dir))
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        had_entries = true;
        entry_name_len = strlen(entry->d_name);
        entry_path = (char *)malloc(sizeof(char) * (path_len + entry_name_len + 2));
        if (entry_path == NULL)
        {
            printf("low memory\n");
            cout << "entry count : " << entry_count << endl;
            exit(1);
        }
        strcpy(entry_path, path);
        strcpy(entry_path + path_len, entry->d_name);
        if (!(lstat(entry_path, &s) < 0))
        {
            if (S_ISDIR(s.st_mode))
            {
                if (entry_path[path_len + entry_name_len - 1] != '/')
                {
                    entry_path[path_len + entry_name_len] = '/';
                    entry_path[path_len + entry_name_len + 1] = '\0';
                }
                compute_memory_requirement(entry_path);
            }
            else
            {
                // cout<<"["<<entry_path<<"]"<<endl;
                if (!(S_ISLNK(s.st_mode)))
                {
                    total = path_len + entry_name_len + 1;
                    if (total > max_path_len)
                        max_path_len = total;
                    memory_requirement += total;
                    entry_count++;
                }
            }
        }
        free(entry_path);
    }
    closedir(dir);
    if (!had_entries)
    {
        total = path_len + 1;
        if (total > max_path_len)
            max_path_len = total;
        memory_requirement += total;
        entry_count++;
    }
}

void populate_file_paths()
{
    // do the changes as discussed in classroom session
    struct stat s;
    DIR *dir;
    dir = opendir(entry_buffer);
    if (dir == NULL)
        return;
    struct dirent *entry;
    int path_len;
    int entry_name_len;
    bool had_entries = false;
    path_len = strlen(entry_buffer);
    while (entry = readdir(dir))
    {
        entry_buffer[path_len] = '\0';
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        had_entries = true;
        entry_name_len = strlen(entry->d_name);
        strcpy(entry_buffer + path_len, entry->d_name); // concatenation
        if (!(lstat(entry_buffer, &s) < 0))
        {
            if (!(S_ISLNK(s.st_mode)) && S_ISDIR(s.st_mode))
            {
                if (entry_buffer[path_len + entry_name_len - 1] != '/')
                {
                    entry_buffer[path_len + entry_name_len] = '/';
                    entry_buffer[path_len + entry_name_len + 1] = '\0';
                }
                populate_file_paths();
            }
            else
            {
                if (!(S_ISLNK(s.st_mode)))
                {
                    total_bytes_to_upload += s.st_size;
                    files.push_back({_index, s.st_size});
                    strcpy(file_paths + _index, entry_buffer + len);
                    _index += (path_len + entry_name_len - len + 1);
                }
            }
        }
    }
    closedir(dir);
    if (!had_entries)
    {
        total_bytes_to_upload += 0;
        files.push_back({_index, 0});
        strcpy(file_paths + _index, entry_buffer + len);
        _index += (path_len - len + 1);
    }
}

void setup_globals(char *path)
{
    compute_memory_requirement(path);
    memory_requirement -= (len * entry_count);
    cout << "memory_requirement : " << memory_requirement << endl;
    cout << "max path length : " << max_path_len << endl;
    cout << "entry count : " << entry_count << endl;
    file_paths = (char *)malloc(sizeof(char) * memory_requirement);
    entry_buffer = (char *)malloc(sizeof(char) * (max_path_len + 1));
    strcpy(entry_buffer, path);
    _index = 0;
    total_bytes_to_upload = 0;
    populate_file_paths();
    free(entry_buffer);
    populate_work_vector();
    /*for(auto p:files)
    {
    printf("%s (%lu)\n",file_paths+p.first,p.second);
    }
    printf("Bytes to upload (%lu)\n",total_bytes_to_upload);
    // study fft-client.cpp
    printf("base path is : %s\n",path);*/
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        cout << "Usage : [fft-client server port source target overwrite]" << endl;
        return EXIT_FAILURE;
    }
    // validate arguments
    char *server = argv[1];
    int port = atoi(argv[2]);
    char *source = argv[3];
    char *target = argv[4];
    char *overwrite = argv[5];
    int error;
    server_socket_descriptor = -1;
    error = fft_connect(server, port);
    if (error != SUCCESS)
    {
        write_error(error);
        return EXIT_FAILURE;
    }
    error = copy_to_remote(source, target, *overwrite);
    if (error != SUCCESS)
    {
        write_error(error);
    }
    else
    {
        cout << "File transferred" << endl;
    }
    fft_close_connection();
    static_assert(sizeof(handshake_request_packet) == 8192, "struct _handshake_request_packet is not of 8192");
    static_assert(sizeof(request_packet) == 8192, "struct _request_packet is not of 8192");

    return EXIT_SUCCESS;
}