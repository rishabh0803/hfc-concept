// Uses condition variable per consumer and uses signal to notify each consumer


#include <stdio.h>
#include <pthread.h>
#include <cstdlib>

struct mdm
{
    int num;
};

void set_num(struct mdm *m, int num)
{
    m->num = num;
}

int get_num(struct mdm *m)
{
    return m->num;
}

// =====================================================
struct sync_t
{
    pthread_mutex_t *mtx;
    pthread_cond_t *producer_condition_var;
    pthread_cond_t *consumer_condition_var;
    int number_of_consumers;
    bool produce;
    bool *consume;
    int total_consumed;
};

struct pargs
{
    struct mdm *m;
    struct sync_t *sync;
};

struct cargs
{
    struct mdm *m;
    struct sync_t *sync;
    int identifier;
};

// =====================================================
void *produce(void *v)
{
    struct pargs *pargs;
    pargs = (struct pargs *)v;

    struct mdm *m;
    int i;

    m = pargs->m;
    pthread_mutex_lock(pargs->sync->mtx);
    for (i = 201; i <= 210; ++i)
    {
        while (pargs->sync->produce == false)
            pthread_cond_wait(pargs->sync->producer_condition_var, pargs->sync->mtx);
        set_num(m, i);
        printf("Producer produced %d\n", i);
        pargs->sync->total_consumed = 0;
        pargs->sync->produce = false;

        for (int e = 0; e < pargs->sync->number_of_consumers; e++)
        {
            pargs->sync->consume[e] = true;
            pthread_cond_signal(&pargs->sync->consumer_condition_var[e]);
        }
    }
    pthread_mutex_unlock(pargs->sync->mtx);
    return NULL;
}

// =====================================================
void *consume(void *v)
{
    struct cargs *cargs;
    cargs = (struct cargs *)v;

    int consumer_identifer = cargs->identifier;

    struct mdm *m;
    int j, k;
    m = cargs->m;
    pthread_mutex_lock(cargs->sync->mtx);

    for (j = 1; j <= 10; ++j)
    {
        while (cargs->sync->consume[consumer_identifer - 1] == false)
            pthread_cond_wait(&cargs->sync->consumer_condition_var[consumer_identifer - 1], cargs->sync->mtx);

        k = get_num(m);
        printf("Consumer %d consumed %d\n", consumer_identifer, k);
        cargs->sync->total_consumed++;
        cargs->sync->consume[consumer_identifer - 1] = false;

        if (cargs->sync->total_consumed == cargs->sync->number_of_consumers)
        {
            cargs->sync->produce = true;
            pthread_cond_signal(cargs->sync->producer_condition_var);
        }
    }

    pthread_mutex_unlock(cargs->sync->mtx);
    return NULL;
}

// =====================================================
int main()
{
    struct mdm m;

    int num_consumers = 5;
    struct pargs pargs;
    struct cargs cargs[num_consumers];

    pthread_mutex_t mtx;
    pthread_t pid, cid[num_consumers];

    pthread_cond_t producer_condition_var;
    pthread_cond_t consumer_condition_var[num_consumers];

    int i1, i2[num_consumers], e;
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&producer_condition_var, NULL);
    for (e = 0; e < num_consumers; e++)
    {
        pthread_cond_init(&consumer_condition_var[e], NULL);
    }

    struct sync_t sync;
    sync.mtx = &mtx;
    sync.producer_condition_var = &producer_condition_var;
    sync.consumer_condition_var = consumer_condition_var;
    sync.number_of_consumers = num_consumers;

    sync.produce = true;
    sync.consume = (bool *)malloc(num_consumers * sizeof(bool));
    for (e = 0; e < num_consumers; e++)
    {
        sync.consume[e] = false;
    }

    pargs.m = &m;
    pargs.sync = &sync;

    for (e = 0; e < num_consumers; e++)
    {
        cargs[e].m = &m;
        cargs[e].sync = &sync;
        cargs[e].identifier = e + 1;
    }

    printf("Producer and Consumers are starting their work...\n");
    i1 = pthread_create(&pid, NULL, produce, (void *)&pargs);

    for (e = 0; e < num_consumers; e++)
    {
        i2[e] = pthread_create(&cid[e], NULL, consume, (void *)&cargs[e]);
    }

    pthread_join(pid, NULL);
    for (e = 0; e < num_consumers; e++)
    {
        pthread_join(cid[e], NULL);
    }

    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&producer_condition_var);
    for (e = 0; e < num_consumers; e++)
    {
        pthread_cond_destroy(&consumer_condition_var[e]);
    }

    return 0;
}