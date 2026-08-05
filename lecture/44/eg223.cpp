#include <iostream>
#include <linux/io_uring.h>

using namespace std;

/*
struct io_uring_sqe
{
    u8 opcode 	u8 flags	we are not trying to understand this right now 	u16 ioprio we are not discussing what exactly to assign
    s32 fd
    u64 offset	to start I/O from which position
    u64 addr	base address of iovec/buffer
    u32 len		number of iovec/buffers
    union {
        we are not trying to understand this right now
    }
    u64 user_data
    union {
        we are not trying to understand this right now
    }
}
structure created from the above will be of 64 bytes (memory aligned)
*/

int main()
{
    cout << sizeof(struct io_uring_sqe) << endl;
    return 0;
}