// Assumption
// 1. Number of elements is atlease more than 2 * cores(thread)

#include <iostream>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <thread>

using namespace std;

struct targs
{
    int *data;
    int start;
    int end;
    int identifier;
    uint64_t *result;
};

void *sum(void *args)
{
    struct targs *targs = (struct targs *)args;
    uint64_t result = 0;
    for (int i = targs->start; i <= targs->end; i++)
    {
        result += targs->data[i];
    }

    targs->result[targs->identifier] = result;

    return NULL;
}

int main()
{
    int number_of_elements = 1000000000;
    int *x = new int[number_of_elements];

    srand(time(NULL));

    for (int i = 0; i < number_of_elements; i++)
    {
        x[i] = rand();
    }

    auto cores = std::thread::hardware_concurrency();
    int number_of_threads = cores;

    // Approach 1 : Compute some via multi threading
    auto start = std::chrono::high_resolution_clock::now();

    pthread_t *t = new pthread_t[number_of_threads];
    uint64_t *result_array = new uint64_t[number_of_threads];
    struct targs *targs = new struct targs[number_of_threads];
    uint64_t final_result = 0;

    int batch_size = number_of_elements / number_of_threads;
    if (number_of_elements % number_of_threads != 0)
        batch_size++;

    int start_at = 0;
    int end_at = 0;
    for (int i = 0; i < number_of_threads; i++)
    {
        end_at = start_at + batch_size - 1;

        targs[i].data = x;
        targs[i].identifier = i;
        targs[i].start = start_at;
        targs[i].end = end_at;
        targs[i].result = result_array;

        start_at = end_at + 1;

        pthread_create(&t[i], NULL, sum, (void *)&targs[i]);
    }

    for (int i = 0; i < number_of_threads; i++)
    {
        pthread_join(t[i], NULL);
    }

    for (int i = 0; i < number_of_threads; i++)
    {
        final_result += result_array[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff1 = end - start;

    // Approach 2: Compute some via single loop
    start = std::chrono::high_resolution_clock::now();

    uint64_t final_result2 = 0;
    for (int i = 0; i < number_of_elements; i++)
    {
        final_result2 += x[i];
    }

    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff2 = end - start;

    // Printing result
    cout << "Time taken in Approach 1 " << diff1.count() << " Sum : " << final_result << endl;
    cout << "Time taken in Approach 2 " << diff2.count() << " Sum : " << final_result2 << endl;

    return 0;
}
