/*
 * Server using io_uring
 */

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

using namespace std;

// ------------------------------- Operation Type for io_uring operations -------------------------------
typedef enum _operation_type
{
    ACCEPT_OPERATION,
    READ_OPERATION,
    WRITE_OPERATION
} operation_type_t;

// ------------------------------- User Data Structure for io_uring operations -------------------------------
struct user_data
{
    int file_descriptor;
    operation_type_t operation_type;
    char buffer[4096];
};

// ------------------------------- System Call Wrappers for io_uring -------------------------------
int io_uring_setup(__u32 sq_entries, struct io_uring_params *p)
{
    return (int)syscall(__NR_io_uring_setup, sq_entries, p);
}

int io_uring_enter(int ring_file_descriptor, unsigned int to_submit, unsigned int min_complete, unsigned int flags)
{
    return (int)syscall(__NR_io_uring_enter, ring_file_descriptor, to_submit, min_complete, flags, NULL, 0);
}

// ------------------------------- Helper Functions for io_uring -------------------------------

// ------------------------------- Main Starts -------------------------------
int main()
{
    int server_file_descriptor;
    int client_file_descriptor;
    char buffer[1025];
    struct sockaddr_in server_sockaddr;

    // Create server socket
    server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_file_descriptor < 0)
    {
        printf("Unable to setup socket\n");
        return EXIT_FAILURE;
    }

    // Setup server address structure
    bzero(&server_sockaddr, sizeof(server_sockaddr));
    server_sockaddr.sin_port = htons(3500);
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_file_descriptor, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) < 0)
    {
        printf("Unable to bind socket to port 3500\n");
        close(server_file_descriptor);
        return EXIT_FAILURE;
    }

    // Put server socket in listening mode
    if (listen(server_file_descriptor, SOMAXCONN) < 0)
    {
        printf("Unable to put socket in listening mode\n");
        close(server_file_descriptor);
        return EXIT_FAILURE;
    }

    // --------------- IO_URING Setup Starts ---------------
    int ring_file_descriptor;
    struct io_uring_params p;
    memset(&p, 0, sizeof(p));

    ring_file_descriptor = io_uring_setup(8192, &p);
    if (ring_file_descriptor < 0)
    {
        close(server_file_descriptor);
        return EXIT_FAILURE;
    }

    __u32 size_of_sqring = p.sq_off.array + p.sq_entries * sizeof(unsigned int);
    __u32 size_of_cqring = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);
    size_t mmap_size = (size_of_sqring > size_of_cqring) ? size_of_sqring : size_of_cqring;
    void *sq_ptr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, ring_file_descriptor, IORING_OFF_SQ_RING);
    if (sq_ptr == MAP_FAILED)
    {
        close(server_file_descriptor);
        close(ring_file_descriptor);
        cout << "Mapping failed" << endl;
        return EXIT_FAILURE;
    }
    // --------------- IO_URING Setup Starts ---------------

    // Setting up submission queue entries
    size_t mmap_size_of_submission_queue_entries = sizeof(struct io_uring_sqe) * p.sq_entries;
    struct io_uring_sqe *submission_queue_entries;
    submission_queue_entries = (struct io_uring_sqe *)mmap(NULL, mmap_size_of_submission_queue_entries, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, ring_file_descriptor, IORING_OFF_SQES);
    if (submission_queue_entries == MAP_FAILED)
    {
        munmap(sq_ptr, mmap_size);
        close(server_file_descriptor);
        close(ring_file_descriptor);
        printf("Mapping for submission queue entries failed\n");
        return EXIT_FAILURE;
    }

    // Adding an accept operation to the submission queue
    __u32 submission_queue_tail = *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail));
    __u32 submission_queue_ring_mask = *((__u32 *)(((char *)sq_ptr) + p.sq_off.ring_mask));
    __u32 *array = ((__u32 *)(((char *)sq_ptr) + p.sq_off.array));
    __u32 index = submission_queue_tail & submission_queue_ring_mask;
    array[index] = index;

    struct user_data *u_data;
    u_data = (struct user_data *)malloc(sizeof(struct user_data));
    u_data->file_descriptor = server_file_descriptor;
    u_data->operation_type = ACCEPT_OPERATION;

    struct io_uring_sqe *sq_entry = &submission_queue_entries[index];
    memset(sq_entry, 0, sizeof(struct io_uring_sqe));
    sq_entry->opcode = IORING_OP_ACCEPT;
    sq_entry->fd = server_file_descriptor;
    sq_entry->user_data = (__u64)u_data;

    submission_queue_tail = (submission_queue_tail + 1) & submission_queue_ring_mask;
    if (*((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) != submission_queue_tail)
        *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) = submission_queue_tail;

    // Setting up completion queue entries
    void *cq_ptr = sq_ptr;
    struct io_uring_cqe *completion_queue_entries;
    __u32 completion_queue_head = *((__u32 *)(((char *)cq_ptr) + p.cq_off.head));
    __u32 completion_queue_ring_mask = *((__u32 *)(((char *)cq_ptr) + p.cq_off.ring_mask));
    completion_queue_entries = (struct io_uring_cqe *)(((char *)cq_ptr) + p.cq_off.cqes);
    struct io_uring_cqe *cq_entry;
    struct user_data *u_d;

    // Server is ready to accept requests
    cout << "Server is ready to accept request on port 3500" << endl;

    int r;
    int entries_added = 1;
    int bytes_read = 0;

    while (1)
    {
        r = io_uring_enter(ring_file_descriptor, entries_added, 1, IORING_ENTER_GETEVENTS);
        entries_added = 0;

        while (1)
        {
            if (completion_queue_head == *((__u32 *)(((char *)cq_ptr) + p.cq_off.tail)))
                break;

            cq_entry = &completion_queue_entries[completion_queue_head & completion_queue_ring_mask];
            u_d = (struct user_data *)cq_entry->user_data;

            if (u_d->operation_type == ACCEPT_OPERATION)
            {
                printf("Accept part completed\n");
                client_file_descriptor = cq_entry->res;

                // Adding a read operation to the submission queue
                submission_queue_tail = *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail));
                submission_queue_ring_mask = *((__u32 *)(((char *)sq_ptr) + p.sq_off.ring_mask));
                array = ((__u32 *)(((char *)sq_ptr) + p.sq_off.array));
                index = submission_queue_tail & submission_queue_ring_mask;
                array[index] = index;

                u_data = (struct user_data *)malloc(sizeof(struct user_data));
                u_data->file_descriptor = client_file_descriptor;
                u_data->operation_type = READ_OPERATION;

                sq_entry = &submission_queue_entries[index];
                memset(sq_entry, 0, sizeof(struct io_uring_sqe));
                sq_entry->opcode = IORING_OP_READ;
                sq_entry->fd = client_file_descriptor;
                sq_entry->flags = 0;
                sq_entry->off = 0;
                sq_entry->len = 4096;
                sq_entry->addr = (__u64)u_data->buffer;
                sq_entry->user_data = (__u64)u_data;

                submission_queue_tail = (submission_queue_tail + 1) & submission_queue_ring_mask;
                if (*((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) != submission_queue_tail)
                    *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) = submission_queue_tail;

                ++entries_added;

                // Adding a Accept operation to the submission queue
                submission_queue_tail = *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail));
                submission_queue_ring_mask = *((__u32 *)(((char *)sq_ptr) + p.sq_off.ring_mask));
                array = ((__u32 *)(((char *)sq_ptr) + p.sq_off.array));
                index = submission_queue_tail & submission_queue_ring_mask;
                array[index] = index;

                u_data = (struct user_data *)malloc(sizeof(struct user_data));
                u_data->file_descriptor = server_file_descriptor;
                u_data->operation_type = ACCEPT_OPERATION;

                sq_entry = &submission_queue_entries[index];
                memset(sq_entry, 0, sizeof(struct io_uring_sqe));
                sq_entry->opcode = IORING_OP_ACCEPT;
                sq_entry->fd = server_file_descriptor;
                sq_entry->flags = 0;
                sq_entry->off = 0;
                sq_entry->user_data = (__u64)u_data;
                submission_queue_tail = (submission_queue_tail + 1) & submission_queue_ring_mask;
                if (*((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) != submission_queue_tail)
                    *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) = submission_queue_tail;

                ++entries_added;
            }
            else if (u_d->operation_type == READ_OPERATION)
            {
                printf("Read part completed\n");
                bytes_read = cq_entry->res;
                if (bytes_read == 0)
                    close(u_d->file_descriptor);
                else
                {
                    // process data against u_d->buffer (not doing anything now)

                    // Adding a write operation to the submission queue
                    submission_queue_tail = *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail));
                    submission_queue_ring_mask = *((__u32 *)(((char *)sq_ptr) + p.sq_off.ring_mask));
                    array = ((__u32 *)(((char *)sq_ptr) + p.sq_off.array));
                    index = submission_queue_tail & submission_queue_ring_mask;
                    array[index] = index;

                    u_data = (struct user_data *)malloc(sizeof(struct user_data));
                    u_data->file_descriptor = u_d->file_descriptor;
                    u_data->operation_type = WRITE_OPERATION;

                    sq_entry = &submission_queue_entries[index];
                    memset(sq_entry, 0, sizeof(struct io_uring_sqe));
                    sq_entry->opcode = IORING_OP_WRITE;
                    sq_entry->fd = u_d->file_descriptor;
                    sq_entry->flags = 0;
                    sq_entry->off = 0;
                    sq_entry->len = bytes_read;
                    memcpy(u_data->buffer, u_d->buffer, bytes_read);
                    sq_entry->addr = (__u64)u_data->buffer;
                    sq_entry->user_data = (__u64)u_data;

                    submission_queue_tail = (submission_queue_tail + 1) & submission_queue_ring_mask;
                    if (*((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) != submission_queue_tail)
                        *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) = submission_queue_tail;

                    ++entries_added;
                }
            }
            else if (u_d->operation_type == WRITE_OPERATION)
            {
                printf("Write part completed\n");

                // Adding a read operation to the submission queue
                submission_queue_tail = *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail));
                submission_queue_ring_mask = *((__u32 *)(((char *)sq_ptr) + p.sq_off.ring_mask));
                array = ((__u32 *)(((char *)sq_ptr) + p.sq_off.array));
                index = submission_queue_tail & submission_queue_ring_mask;
                array[index] = index;

                u_data = (struct user_data *)malloc(sizeof(struct user_data));
                u_data->file_descriptor = u_d->file_descriptor;
                u_data->operation_type = READ_OPERATION;

                sq_entry = &submission_queue_entries[index];
                memset(sq_entry, 0, sizeof(struct io_uring_sqe));
                sq_entry->opcode = IORING_OP_READ;
                sq_entry->fd = u_d->file_descriptor;
                sq_entry->flags = 0;
                sq_entry->off = 0;
                sq_entry->len = 4096;
                sq_entry->addr = (__u64)u_data->buffer;
                sq_entry->user_data = (__u64)u_data;

                submission_queue_tail = (submission_queue_tail + 1) & submission_queue_ring_mask;
                if (*((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) != submission_queue_tail)
                    *((__u32 *)(((char *)sq_ptr) + p.sq_off.tail)) = submission_queue_tail;

                ++entries_added;
            }

            free(u_d);

            completion_queue_head = (completion_queue_head + 1) & completion_queue_ring_mask;
            if (completion_queue_head != *((__u32 *)(((char *)cq_ptr) + p.cq_off.head)))
                *((__u32 *)(((char *)cq_ptr) + p.cq_off.head)) = completion_queue_head;
        }
    }

    // TODO:: Cleanup
    close(server_file_descriptor);
    close(ring_file_descriptor);

    return EXIT_SUCCESS;
}
// ------------------------------- Main Ends -------------------------------
