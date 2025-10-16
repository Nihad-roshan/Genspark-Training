#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_CHILDREN 2
pid_t children[NUM_CHILDREN];


void child_work(int id)
{
    int count = 0; // Each child has its own counter
    while (1)
    {
        printf("Child %d (PID %d): count = %d\n", id, getpid(), count++);
        fflush(stdout); //flushing
        sleep(4);       // sleeping 4 sec slowing counting the numbers
    }
}

// ------------------ SIGNAL HANDLER FOR PARENT ------------------
void handle_signal(int sig)
{
    if (sig == SIGINT)
    {
        printf("\nController: Received SIGINT → stopping all children...\n");
        for (int i = 0; i < NUM_CHILDREN; i++)
        {
            kill(children[i], SIGSTOP); // Stop child so the  counting 
        }
    }
    else if (sig == SIGUSR1)
    {
        printf("Controller: Received SIGUSR1 → resuming all children...\n");
        for (int i = 0; i < NUM_CHILDREN; i++)
        {
            kill(children[i], SIGCONT); // Resume child so the counting
        }
    }
    else if (sig == SIGTERM)
    {
        printf("Controller: Received SIGTERM → terminating all children...\n");
        for (int i = 0; i < NUM_CHILDREN; i++)
        {
            kill(children[i], SIGTERM); // End counting loop
        }

        for (int i = 0; i < NUM_CHILDREN; i++)
        {
            waitpid(children[i], NULL, 0); // Wait for clean exit
        }
        printf("Controller: All children terminated. Exiting.\n");
        exit(0);
    }
}


int main()
{
    struct sigaction sa;
    sa.sa_handler = handle_signal; //function
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // Register signal handlers for parent
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Spawn child processes
    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child runs its counting job
            child_work(i + 1); //here called twice
            exit(0);
        }
        else
        {
            children[i] = pid; // Parent stores PID
        }
    }

    // Print usage info
    printf("Controller PID = %d\n", getpid());
    printf("Use signals:\n");
    printf("  SIGINT (Ctrl+C) -> Stop children\n");
    printf("  kill -SIGUSR1 %d → Resume children\n", getpid());
    printf("  kill -SIGTERM %d → Terminate all\n", getpid());

    // Keep parent waiting for signals
    while (1)
    {
        pause();
    }

    return 0;
}
