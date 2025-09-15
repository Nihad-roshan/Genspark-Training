#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define THREADS 8
#define ITERATIONS 1000000 

static long atomic_counter = 0;
static long mutex_counter = 0;

pthread_mutex_t lock;

typedef struct
{
    int id;
    long iterations;
} worker_arg_t;

void *worker_atomic(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;
    for (long i = 0; i < worker->iterations; i++)
    {
        __atomic_fetch_add(&atomic_counter, 1, __ATOMIC_SEQ_CST);
    }
    return NULL;
}

void *worker_mutex(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;
    for (long i = 0; i < worker->iterations; i++)
    {
        pthread_mutex_lock(&lock);
        mutex_counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}


double elapsed_time(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main()
{
    pthread_t threads[THREADS];
    worker_arg_t args[THREADS];
    struct timespec start, end;
    double time_atomic, time_mutex;

    atomic_counter = 0;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < THREADS; i++)
    {
        args[i].id = i;
        args[i].iterations = ITERATIONS;
        pthread_create(&threads[i], NULL, worker_atomic, &args[i]);
    }
    for (int i = 0; i < THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_atomic = elapsed_time(start, end);

    // Mutex test
    pthread_mutex_init(&lock, NULL);
    mutex_counter = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < THREADS; i++)
    {
        args[i].id = i;
        args[i].iterations = ITERATIONS;
        pthread_create(&threads[i], NULL, worker_mutex, &args[i]);
    }
    for (int i = 0; i < THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_mutex = elapsed_time(start, end);

    printf("Atomic Counter = %ld (expected %ld), time = %.6f seconds\n",atomic_counter, (long)THREADS * ITERATIONS, time_atomic);
    printf("Mutex Counter  = %ld (expected %ld), time = %.6f seconds\n",mutex_counter, (long)THREADS * ITERATIONS, time_mutex);

    pthread_mutex_destroy(&lock);
    return 0;
}
