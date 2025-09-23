#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 1000000

// Shared variables
atomic_int atomic_counter = 0;
int mutex_counter = 0;
pthread_mutex_t lock;

// Worker for atomic counter
void *atomic_worker(void *arg)
{
    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        atomic_fetch_add(&atomic_counter, 1); // Atomic increment
    }
    return NULL;
}

// Worker for mutex counter
void *mutex_worker(void *arg)
{
    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        pthread_mutex_lock(&lock);
        mutex_counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];
    clock_t start, end;

    // --- Atomic Counter Test ---
    start = clock();
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, atomic_worker, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    end = clock();
    printf("Atomic Counter = %d, Time = %.4f sec\n", atomic_counter, (double)(end - start) / CLOCKS_PER_SEC);

    // --- Mutex Counter Test ---
    pthread_mutex_init(&lock, NULL);
    start = clock();
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, mutex_worker, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    end = clock();
    printf("Mutex Counter = %d, Time = %.4f sec\n", mutex_counter, (double)(end - start) / CLOCKS_PER_SEC);

    pthread_mutex_destroy(&lock);
    return 0;
}
/*

Atomic Counter = 4000000, Time = 0.12 sec
Mutex Counter  = 4000000, Time = 0.35 sec

*/

/*

Atomic counter is faster because it avoids the kernel/OS overhead of locking.

Mutex is safer for complex operations, but slower for simple increments.
*/
