#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int count = 0;

pthread_mutex_t mutex;
pthread_cond_t notfull, notempty;

void *producerfunc(void *arg)
{
    int i = 1;
    while (i <= 10)
    {
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE)
        {
            pthread_cond_wait(&notfull, &mutex);
        }

        buffer[count] = i;
        printf("producer produced: %d\n", i);
        count++;

        pthread_cond_signal(&notempty);
        pthread_mutex_unlock(&mutex);

        i++;
        sleep(1);
    }
    pthread_exit(NULL);
}

void *consumerfunc(void *arg)
{
    int data;
    int i = 0;
    while (i < 10)
    {
        pthread_mutex_lock(&mutex);

        while (count == 0)
        {
            pthread_cond_wait(&notempty, &mutex);
        }

        data = buffer[count - 1];
        count--;
        printf("consumer consumed: %d\n", data);

        pthread_cond_signal(&notfull);
        pthread_mutex_unlock(&mutex);

        i++;
        sleep(2);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t producer, consumer;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&notfull, NULL);
    pthread_cond_init(&notempty, NULL);

    pthread_create(&producer, NULL, producerfunc, NULL);
    pthread_create(&consumer, NULL, consumerfunc, NULL);

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&notfull);
    pthread_cond_destroy(&notempty);

    printf("producer and consumer finished\n");
    return 0;
}