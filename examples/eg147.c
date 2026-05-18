// Bug of previous example resolved
#include <stdio.h>
#include <pthread.h>

struct mdm
{
    int num;
    pthread_mutex_t *mtx;
    pthread_cond_t *producer_condition_var;
    pthread_cond_t *consumer_condition_var;
    bool produce; // for should produce or not
};
void set_num(struct mdm *m, int num)
{
    m->num = num;
    printf("Produced : %d\n", m->num);
}
int get_num(struct mdm *m)
{
    printf("Consumed : %d\n", m->num);
    return m->num;
}
void *produce(void *v)
{
    struct mdm *m;
    int i;
    m = (struct mdm *)v;
    pthread_mutex_lock(m->mtx);
    for (i = 201; i <= 250; ++i)
    {
        while (m->produce == false)
            pthread_cond_wait(m->producer_condition_var, m->mtx);
        set_num(m, i); // production in action
        m->produce = false;
        pthread_cond_signal(m->consumer_condition_var);
    }
    pthread_mutex_unlock(m->mtx);
    return NULL;
}
void *consume(void *v)
{
    struct mdm *m;
    int j, k;
    m = (struct mdm *)v;
    pthread_mutex_lock(m->mtx);
    for (j = 1; j <= 50; ++j)
    {
        while (m->produce == true)
            pthread_cond_wait(m->consumer_condition_var, m->mtx);
        k = get_num(m); // consumption in action
        m->produce = true;
        pthread_cond_signal(m->producer_condition_var);
    }
    pthread_mutex_unlock(m->mtx);
    return NULL;
}
int main()
{
    struct mdm m;
    pthread_mutex_t mtx;
    pthread_t pid, cid;
    pthread_cond_t producer_condition_var;
    pthread_cond_t consumer_condition_var;
    int i1, i2;
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&producer_condition_var, NULL);
    pthread_cond_init(&consumer_condition_var, NULL);
    m.mtx = &mtx;
    m.producer_condition_var = &producer_condition_var;
    m.consumer_condition_var = &consumer_condition_var;
    m.produce = true;
    i1 = pthread_create(&pid, NULL, produce, (void *)&m);
    i2 = pthread_create(&cid, NULL, consume, (void *)&m);
    pthread_join(pid, NULL);
    pthread_join(cid, NULL);
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&producer_condition_var);
    pthread_cond_destroy(&consumer_condition_var);
    return 0;
}
