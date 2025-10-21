#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define numberofthreads 3

void *function(void *arg)
{
    int thread_id=*((int *)arg);

    for(int i=1; i<=5; i++)
    {
        printf("Thread %d: iteration %d\n",thread_id,i);
        sleep(1);
    }

    printf("Thread %d finished\n",thread_id);
    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[numberofthreads];
    int thread_id[numberofthreads];

    for (int i = 0; i < numberofthreads; i++)
    {
        thread_id[i] = i + 1;
        if (pthread_create(&threads[i], NULL, function, &thread_id[i]) != 0)
        {
            perror("pthread_create");
            exit(1);
        }
    }

    for (int i = 0; i < numberofthreads; i++)
    {
        pthread_join(threads[i],NULL);
    }

    printf("All threads are completed \n");
    return 0;
}