#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define WORKERS 6
#define ITERATIONS 5

int main()
{
    int shmid;
    int *counters;

    shmid = shmget(IPC_PRIVATE, WORKERS * sizeof(int), IPC_CREAT | 0666); // creating a shared memory segment
    if (shmid < 0)
    {
        perror("shmget failed -> creation of shared memory segment failed!");
        exit(1);
    }

    counters = (int *)shmat(shmid, NULL, 0); // attaching shared memory to master process

    if (counters == (int *)-1)
    {
        perror("shmat -> attachin shared memory to master processor failed!");
        exit(1);
    }

    for (int i = 0; i < WORKERS; i++)
    {
        counters[i] = 0; // initializing to zer0
    }

    for (int i = 0; i < WORKERS; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
            exit(1);
        }
        else if (pid == 0)
        { // inside child
            for (int j = 0; j < ITERATIONS; j++)
            {
                counters[i] = counters[i] + 1;
            }

            shmdt(counters); // detaching counters from shared memory
            exit(0);
        }
    }

    for (int i = 0; i < WORKERS; i++)
    { // parent  waiting for all childs to complete the work
        wait(NULL);
    }

    int total = 0;
    for (int i = 0; i < WORKERS; i++)
    {
        printf("worker %d counter =%d\n", i, counters[i]);
        total = total + counters[i];
    }
    printf("Total count of all workers = %d\n", total);

    shmdt(counters);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}