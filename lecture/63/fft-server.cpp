#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <linux/io_uring.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgen.h>

using namespace std;

#define HANDSHAKE_PACKET 'H'
#define FILE_DATA_PACKET 'D'

typedef struct _handshake_request_packet
{
    char packet_type;
    char buffer[7926];
    char filename[256];
    char overwrite;
    uint64_t source_file_size;
} handshake_request_packet;

typedef struct _handshake_response_packet
{
    char can_copy;
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
    char buffer[8192]; // need to think about this
} response_packet;

typedef enum _operation_type
{
    ACCEPT_OPERATION,
    READ_OPERATION,
    WRITE_OPERATION
} operation_type_t;

struct user_data
{
    int file_descriptor;
    int socket_file_descriptor;
    uint64_t source_file_size;
    uint32_t number_of_chunks;
    uint32_t chunks_left_to_write;
    operation_type_t operation_type;
    char buffer[8192];
};

typedef enum _enums
{
    TCP_SOCKET_ERROR = 1001,
    TCP_SOCKET_BIND_ERROR,
    TCP_SOCKET_LISTEN_ERROR,
    MY_URING_SETUP_ERROR,
    MY_URING_SQ_MAP_ERROR,
    MY_URING_MAP_FAILED,
    MEMORY_HEAP_ERROR,
    SUCCESS
} enums;

struct global_wrapper
{
    enums error;
    struct io_uring_params uring_params;
    void *sq_ptr;
    void *cq_ptr;
    struct io_uring_sqe *submission_queue_entries;
    struct io_uring_cqe *completion_queue_entries;
    size_t mmap_size;
    size_t mmap_size_of_submission_queue_entries;
} global_s;

// ------------------------- File operations -------------------------
int create_file(char *path, uint64_t file_size)
{
    int file_descriptor = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (file_descriptor < 0)
        return -1;
    if (ftruncate(file_descriptor, file_size) < 0)
    {
        close(file_descriptor);
        return -1;
    }
    return file_descriptor;
}

char can_copy(char *target, char *why_cannot_copy, char overwrite)
{
    char *target_path = strdup(target);
    *why_cannot_copy = '\0';
    if (!target_path)
    {
        strcpy(why_cannot_copy, "LOW SERVER MEMORY");
        return (char)0;
    }

    struct stat s;
    if (stat(target_path, &s) == 0 && S_ISREG(s.st_mode) && overwrite != 'Y')
    {
        strcpy(why_cannot_copy, "File exist");
        free(target_path);
        return (char)0;
    }

    char *parent_path = dirname(target_path);
    if (stat(parent_path, &s) < 0)
    {
        strcpy(why_cannot_copy, "Target directory does not exist");
        free(target_path);
        return (char)0;
    }

    if (!(S_ISDIR(s.st_mode) != 0))
    {
        strcpy(why_cannot_copy, "Target directory is not a directory, it is a file");
        free(target_path);
        return (char)0;
    }

    if (access(target_path, X_OK | W_OK) != 0)
    {
        strcpy(why_cannot_copy, "No permission to create file on the path");
        free(target_path);
        return (char)0;
    }

    free(target_path);

    return (char)1;
}

// ------------------------- Error logging -------------------------
void log_error()
{
    // pick from global_s, use switch case and print message
    switch (global_s.error)
    {
    case TCP_SOCKET_ERROR:
        cout << "Unable to create socket" << endl;
        break;
    case TCP_SOCKET_BIND_ERROR:
        cout << "Unable to bind socket" << endl;
        break;
    case TCP_SOCKET_LISTEN_ERROR:
        cout << "Unable to put socket in listening mode" << endl;
        break;
    case MY_URING_SETUP_ERROR:
        cout << "io uring setup error" << endl;
        break;
    case MY_URING_MAP_FAILED:
        cout << "unable to create map for io uring" << endl;
        break;
    case MY_URING_SQ_MAP_ERROR:
        cout << "unable to create map for submission queue entries" << endl;
        break;
    case MEMORY_HEAP_ERROR:
        cout << "low memory" << endl;
        break;
    }
}

// ------------------------- TCP server configuration -------------------------
int tcp_configure_server(int port)
{
    int server_file_descriptor;
    struct sockaddr_in server_sockaddr;

    server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_file_descriptor < 0)
    {
        global_s.error = TCP_SOCKET_ERROR;
        return -1;
    }

    bzero(&server_sockaddr, sizeof(server_sockaddr));
    server_sockaddr.sin_port = htons(port);
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_file_descriptor, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) < 0)
    {
        global_s.error = TCP_SOCKET_BIND_ERROR;
        close(server_file_descriptor);
        return -1;
    }

    if (listen(server_file_descriptor, SOMAXCONN) < 0)
    {
        global_s.error = TCP_SOCKET_LISTEN_ERROR;
        close(server_file_descriptor);
        return -1;
    }

    return server_file_descriptor;
}

// ------------------------- io_uring setup -------------------------
int my_uring_setup(int queue_depth)
{
    memset(&(global_s.uring_params), 0, sizeof(struct io_uring_params)); // set all bytes to 0
    int ring_file_descriptor = (int)syscall(__NR_io_uring_setup, queue_depth, &(global_s.uring_params));
    if (ring_file_descriptor < 0)
    {
        global_s.error = MY_URING_SETUP_ERROR;
        return -1;
    }
    __u32 size_of_sqring = global_s.uring_params.sq_off.array + global_s.uring_params.sq_entries * sizeof(unsigned int);
    __u32 size_of_cqring = global_s.uring_params.cq_off.cqes + global_s.uring_params.cq_entries * sizeof(struct io_uring_cqe);
    global_s.mmap_size = (size_of_sqring > size_of_cqring) ? size_of_sqring : size_of_cqring;
    global_s.sq_ptr = mmap(NULL, global_s.mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, ring_file_descriptor, IORING_OFF_SQ_RING);
    if (global_s.sq_ptr == MAP_FAILED)
    {
        close(ring_file_descriptor);
        global_s.error = MY_URING_MAP_FAILED;
        return -1;
    }

    global_s.mmap_size_of_submission_queue_entries = sizeof(struct io_uring_sqe) * global_s.uring_params.sq_entries;
    global_s.submission_queue_entries = (struct io_uring_sqe *)mmap(NULL, global_s.mmap_size_of_submission_queue_entries, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, ring_file_descriptor, IORING_OFF_SQES);
    if (global_s.submission_queue_entries == MAP_FAILED)
    {
        munmap(global_s.sq_ptr, global_s.mmap_size);
        printf("Mapping for submission queue entries failed\n");
        close(ring_file_descriptor);
        global_s.error = MY_URING_SQ_MAP_ERROR;
        return -1;
    }

    global_s.cq_ptr = global_s.sq_ptr;
    //__u32 completion_queue_head=*((__u32 *)(((char *)cq_ptr)+p.cq_off.head));
    //__u32 completion_queue_ring_mask=*((__u32 *)(((char *)cq_ptr)+p.cq_off.ring_mask));
    global_s.completion_queue_entries = (struct io_uring_cqe *)(((char *)(global_s.cq_ptr)) + global_s.uring_params.cq_off.cqes);

    return ring_file_descriptor;
}

// ----
int my_uring_enque_accept_task(int ring_file_descriptor, int server_file_descriptor)
{
    __u32 submission_queue_tail = *((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.tail));
    __u32 submission_queue_ring_mask = *((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.ring_mask));
    __u32 *array = ((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.array));
    __u32 index = submission_queue_tail & submission_queue_ring_mask;
    array[index] = index;

    struct user_data *u_data;
    u_data = (struct user_data *)malloc(sizeof(struct user_data));
    if (u_data == NULL)
    {
        global_s.error = MEMORY_HEAP_ERROR;
        return -1;
    }

    u_data->socket_file_descriptor = server_file_descriptor;
    u_data->operation_type = ACCEPT_OPERATION;

    struct io_uring_sqe *sq_entry = &global_s.submission_queue_entries[index];
    memset(sq_entry, 0, sizeof(struct io_uring_sqe));
    sq_entry->opcode = IORING_OP_ACCEPT;
    sq_entry->fd = server_file_descriptor;
    sq_entry->flags = 0;
    sq_entry->off = 0;
    sq_entry->user_data = (__u64)u_data;

    submission_queue_tail = (submission_queue_tail + 1) & submission_queue_ring_mask;
    if (*((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.tail)) != submission_queue_tail)
    {
        *((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.tail)) = submission_queue_tail;
    }

    return SUCCESS;
}

// --------
int my_uring_get_events(int ring_file_descriptor, int entries_added, int minimum_completed)
{
    return (int)syscall(__NR_io_uring_enter, ring_file_descriptor, entries_added, minimum_completed, IORING_ENTER_GETEVENTS, NULL, 0);
}

int my_uring_is_completion_queue_empty(int ring_file_descriptor)
{
    return *((__u32 *)(((char *)(global_s.cq_ptr)) + global_s.uring_params.cq_off.head)) == *((__u32 *)(((char *)global_s.cq_ptr) + global_s.uring_params.cq_off.tail));
}

struct io_uring_cqe *my_uring_get_cq_entry(int ring_file_descriptor)
{
    __u32 completion_queue_head = *((__u32 *)(((char *)(global_s.cq_ptr)) + global_s.uring_params.cq_off.head));
    __u32 head = completion_queue_head;
    __u32 completion_queue_ring_mask = *((__u32 *)(((char *)(global_s.cq_ptr)) + global_s.uring_params.cq_off.ring_mask));

    completion_queue_head = (completion_queue_head + 1) & completion_queue_ring_mask;
    if (completion_queue_head != *((__u32 *)(((char *)(global_s.cq_ptr)) + global_s.uring_params.cq_off.head)))
    {
        *((__u32 *)(((char *)(global_s.cq_ptr)) + global_s.uring_params.cq_off.head)) = completion_queue_head;
    }

    return &(global_s.completion_queue_entries[head]);
}

int my_uring_get_operation(struct io_uring_cqe *cq_entry)
{
    return ((struct user_data *)cq_entry->user_data)->operation_type;
}

int my_uring_enque_network_read_task(int ring_file_descriptor, struct user_data *u_data, int buffer_size)
{
    __u32 submission_queue_tail = *((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.tail));
    __u32 submission_queue_ring_mask = *((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.ring_mask));
    __u32 *array = ((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.array));

    int index = submission_queue_tail & submission_queue_ring_mask;
    array[index] = index;

    struct io_uring_sqe *sq_entry = &(global_s.submission_queue_entries[index]);
    memset(sq_entry, 0, sizeof(struct io_uring_sqe));
    sq_entry->opcode = IORING_OP_RECV;
    sq_entry->fd = u_data->socket_file_descriptor;
    sq_entry->flags = 0;
    sq_entry->off = 0;
    sq_entry->msg_flags = MSG_WAITALL;
    sq_entry->len = buffer_size;
    sq_entry->addr = (__u64)u_data->buffer;
    sq_entry->user_data = (__u64)u_data;

    submission_queue_tail = (submission_queue_tail + 1) & submission_queue_ring_mask;
    if (*((__u32 *)(((char *)(global_s.sq_ptr) + global_s.uring_params.sq_off.tail))) != submission_queue_tail)
    {
        *((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.tail)) = submission_queue_tail;
    }

    return SUCCESS;
}
int my_uring_enque_network_write_task(int ring_file_descriptor, struct user_data *sq_user_data, int bytes_to_write)
{
    __u32 submission_queue_tail = *((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.tail));
    __u32 submission_queue_ring_mask = *((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.ring_mask));
    __u32 *array = ((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.array));

    int index = submission_queue_tail & submission_queue_ring_mask;
    array[index] = index;

    sq_user_data->operation_type = WRITE_OPERATION;
    struct io_uring_sqe *sq_entry = &(global_s.submission_queue_entries[index]);
    memset(sq_entry, 0, sizeof(struct io_uring_sqe));
    sq_entry->opcode = IORING_OP_WRITE;
    sq_entry->fd = sq_user_data->socket_file_descriptor;
    sq_entry->flags = 0;
    sq_entry->off = 0;
    sq_entry->len = bytes_to_write;
    sq_entry->addr = (__u64)sq_user_data->buffer;
    sq_entry->user_data = (__u64)sq_user_data;
    submission_queue_tail = (submission_queue_tail + 1) & submission_queue_ring_mask;

    if (*((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.tail)) != submission_queue_tail)
    {
        *((__u32 *)(((char *)(global_s.sq_ptr)) + global_s.uring_params.sq_off.tail)) = submission_queue_tail;
    }

    return SUCCESS;
}

int main()
{
    int server_file_descriptor;
    int client_file_descriptor;

    server_file_descriptor = tcp_configure_server(3500);
    if (server_file_descriptor < 0)
    {
        log_error();
        return EXIT_FAILURE;
    }

    int ring_file_descriptor;
    ring_file_descriptor = my_uring_setup(8192);
    if (ring_file_descriptor < 0)
    {
        close(server_file_descriptor);
        log_error();
        return EXIT_FAILURE;
    }

    if (my_uring_enque_accept_task(ring_file_descriptor, server_file_descriptor) < 0)
    {
        return EXIT_FAILURE;
    }

    struct io_uring_cqe *cq_entry;
    cout << "Server is ready to accept request on port 3500" << endl;

    int entries_added = 1;
    int r;
    while (1)
    {
        r = my_uring_get_events(ring_file_descriptor, entries_added, 1);
        entries_added = 0;
        int bytes_read;
        int bytes_to_write;
        struct user_data *cq_user_data, *sq_user_data;
        char *buffer_to_write;
        // extraction from completion queue starts here
        while (1)
        {
            if (my_uring_is_completion_queue_empty(ring_file_descriptor))
                break;
            cq_entry = my_uring_get_cq_entry(ring_file_descriptor);
            cq_user_data = (struct user_data *)cq_entry->user_data;
            if (my_uring_get_operation(cq_entry) == ACCEPT_OPERATION)
            {
                printf("Accept part completed\n");

                // add entry for read operation
                struct user_data *u_data = (struct user_data *)malloc(sizeof(struct user_data));
                u_data->socket_file_descriptor = cq_entry->res;
                u_data->operation_type = READ_OPERATION;
                my_uring_enque_network_read_task(ring_file_descriptor, u_data, 8192);
                ++entries_added;

                // add entry for accept operation
                my_uring_enque_accept_task(ring_file_descriptor, server_file_descriptor);
                ++entries_added;
            }
            else if (my_uring_get_operation(cq_entry) == READ_OPERATION)
            {
                printf("Read part completed\n");

                bytes_read = cq_entry->res;
                if (bytes_read == 0)
                {
                    close(cq_user_data->socket_file_descriptor);
                }
                else
                {
                    // process cq_user_data->buffer on the basis of bytes_read
                    // ?? what processing
                    // cast it to request_packet *
                    // check packet type
                    request_packet *req_pkt = (request_packet *)cq_user_data->buffer;
                    cout << req_pkt->packet_type << " packet" << endl;
                    if (req_pkt->packet_type == HANDSHAKE_PACKET)
                    {
                        handshake_response_packet *handshake_res_pkt;
                        handshake_res_pkt = (handshake_response_packet *)malloc(sizeof(handshake_response_packet));

                        // cast it to handshake request packet
                        handshake_request_packet *handshake_req_pkt;
                        handshake_req_pkt = (handshake_request_packet *)req_pkt;

                        struct stat s;
                        if (stat(handshake_req_pkt->buffer, &s) == 0 && S_ISDIR(s.st_mode))
                        {
                            int last_index = strlen(handshake_req_pkt->buffer) - 1;
                            if (handshake_req_pkt->buffer[last_index] == '/')
                            {
                                strcpy(handshake_req_pkt->buffer + last_index + 1, handshake_req_pkt->filename);
                            }
                            else
                            {
                                handshake_req_pkt->buffer[last_index + 1] = '/';
                                strcpy(handshake_req_pkt->buffer + last_index + 2, handshake_req_pkt->filename);
                            }
                        }

                        handshake_res_pkt->can_copy = can_copy(handshake_req_pkt->buffer, handshake_res_pkt->buffer, handshake_req_pkt->overwrite);

                        int file_descriptor;
                        cout << "Can copy : " << handshake_res_pkt->can_copy << endl;
                        if (handshake_res_pkt->can_copy)
                        {
                            file_descriptor = create_file(handshake_req_pkt->buffer, handshake_req_pkt->source_file_size);
                            if (file_descriptor < 0)
                            {
                                strcpy(handshake_res_pkt->buffer, "Insufficient space, file too large");
                                handshake_res_pkt->can_copy = (char)0;
                            }
                        }

                        // enque write task
                        sq_user_data = (struct user_data *)malloc(sizeof(struct user_data));
                        sq_user_data->socket_file_descriptor = cq_user_data->socket_file_descriptor;
                        memcpy(sq_user_data->buffer, handshake_res_pkt, sizeof(handshake_response_packet));
                        bytes_to_write = sizeof(handshake_response_packet);
                        my_uring_enque_network_write_task(ring_file_descriptor, sq_user_data, bytes_to_write);
                        ++entries_added;

                        if (handshake_res_pkt->can_copy)
                        {
                            struct user_data *u_data = (struct user_data *)malloc(sizeof(struct user_data));
                            u_data->socket_file_descriptor = cq_user_data->socket_file_descriptor;
                            u_data->file_descriptor = file_descriptor;
                            u_data->operation_type = READ_OPERATION;
                            u_data->source_file_size = handshake_req_pkt->source_file_size;
                            u_data->number_of_chunks = u_data->source_file_size / 8187;

                            if (u_data->source_file_size % 8187 != 0)
                                u_data->number_of_chunks++;

                            u_data->chunks_left_to_write = u_data->number_of_chunks;
                            my_uring_enque_network_read_task(ring_file_descriptor, u_data, 8192);
                            ++entries_added;
                            // enque read task, done till here
                        }
                        free(handshake_res_pkt);
                    }
                    else if (req_pkt->packet_type == FILE_DATA_PACKET) // FILE_DATA_PACKET PART starts
                    {
                        uint32_t bytes_to_write = 8187;
                        uint32_t remainder;
                        if (req_pkt->chunk_number == cq_user_data->number_of_chunks)
                        {
                            remainder = cq_user_data->source_file_size % 8187;
                            if (remainder != 0)
                            {
                                bytes_to_write = remainder;
                            }
                        }

                        lseek(cq_user_data->file_descriptor, (req_pkt->chunk_number - 1) * 8187, SEEK_SET);
                        write(cq_user_data->file_descriptor, req_pkt->buffer, bytes_to_write);
                        cq_user_data->chunks_left_to_write--;

                        if (cq_user_data->chunks_left_to_write == 0)
                        {
                            // file upload complete, notify client by enquing a write task
                            response_packet res_pkt;
                            res_pkt.buffer[0] = 'D'; // D for done
                            res_pkt.buffer[1] = '\0';

                            sq_user_data = (struct user_data *)malloc(sizeof(struct user_data));
                            sq_user_data->socket_file_descriptor = cq_user_data->socket_file_descriptor;
                            memcpy(sq_user_data->buffer, &res_pkt, sizeof(response_packet));
                            bytes_to_write = sizeof(response_packet);

                            my_uring_enque_network_write_task(ring_file_descriptor, sq_user_data, bytes_to_write);
                            ++entries_added;
                        }
                        else
                        {
                            // all part not done, enque read task for the client descriptor
                            struct user_data *u_data = (struct user_data *)malloc(sizeof(struct user_data));
                            u_data->socket_file_descriptor = cq_user_data->socket_file_descriptor;
                            u_data->file_descriptor = cq_user_data->file_descriptor;
                            u_data->operation_type = READ_OPERATION;
                            u_data->source_file_size = cq_user_data->source_file_size;
                            u_data->number_of_chunks = cq_user_data->number_of_chunks;
                            u_data->chunks_left_to_write = cq_user_data->chunks_left_to_write;

                            my_uring_enque_network_read_task(ring_file_descriptor, u_data, 8192);
                            ++entries_added;
                        }
                    } // FILE_DATA_PACKET PART COMPLETED
                }
            }
            else if (my_uring_get_operation(cq_entry) == WRITE_OPERATION)
            {
                printf("Write part completed\n");
                // add entry for read operation
                // my_uring_enque_network_read_task(ring_file_descriptor,cq_entry,4096);
                //++entries_added;
            }
            free(cq_user_data);
        }
    }

    return 0;
}