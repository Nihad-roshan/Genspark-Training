#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <stdatomic.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define ITEMS_TO_PRODUCE 10

// Shared buffer
int buffer[BUFFER_SIZE];
int in = 0, out = 0, count = 0;

// Futex helpers
static int futex_wait(atomic_int *addr, int expected)
{
    return syscall(SYS_futex, addr, FUTEX_WAIT, expected, NULL, NULL, 0);
}
static int futex_wake(atomic_int *addr, int n)
{
    return syscall(SYS_futex, addr, FUTEX_WAKE, n, NULL, NULL, 0);
}

// Futex condition replacements
atomic_int not_full = 0;  // tracks if producers should wait
atomic_int not_empty = 0; // tracks if consumers should wait

// Mutex for buffer protection
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *producer(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < ITEMS_TO_PRODUCE; i++)
    {
        int item = i + 1;

        pthread_mutex_lock(&mutex);

        // Wait while buffer is full
        while (count == BUFFER_SIZE)
        {
            printf("Producer %d waiting (buffer full)\n", id);
            atomic_store(&not_full, 1);
            pthread_mutex_unlock(&mutex);
            futex_wait(&not_full, 1); // block until woken
            pthread_mutex_lock(&mutex);
        }

        // Put item
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        printf("Producer %d produced %d (count=%d)\n", id, item, count);

        // Wake up one consumer if waiting
        if (atomic_load(&not_empty) == 1)
        {
            atomic_store(&not_empty, 0);
            futex_wake(&not_empty, 1);
        }

        pthread_mutex_unlock(&mutex);
        usleep(100000); // simulate work
    }

    return NULL;
}

void *consumer(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < ITEMS_TO_PRODUCE; i++)
    {
        pthread_mutex_lock(&mutex);

        // Wait while buffer is empty
        while (count == 0)
        {
            printf("Consumer %d waiting (buffer empty)\n", id);
            atomic_store(&not_empty, 1);
            pthread_mutex_unlock(&mutex);
            futex_wait(&not_empty, 1); // block until woken
            pthread_mutex_lock(&mutex);
        }

        // Take item
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("Consumer %d consumed %d (count=%d)\n", id, item, count);

        // Wake up one producer if waiting
        if (atomic_load(&not_full) == 1)
        {
            atomic_store(&not_full, 0);
            futex_wake(&not_full, 1);
        }

        pthread_mutex_unlock(&mutex);
        usleep(150000); // simulate work
    }

    return NULL;
}

int main()
{
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int ids[NUM_PRODUCERS > NUM_CONSUMERS ? NUM_PRODUCERS : NUM_CONSUMERS];

    // Create producers
    for (int i = 0; i < NUM_PRODUCERS; i++)
    {
        ids[i] = i;
        pthread_create(&producers[i], NULL, producer, &ids[i]);
    }

    // Create consumers
    for (int i = 0; i < NUM_CONSUMERS; i++)
    {
        ids[i] = i;
        pthread_create(&consumers[i], NULL, consumer, &ids[i]);
    }

    // Wait for all
    for (int i = 0; i < NUM_PRODUCERS; i++)
    {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++)
    {
        pthread_join(consumers[i], NULL);
    }

    return 0;
}
