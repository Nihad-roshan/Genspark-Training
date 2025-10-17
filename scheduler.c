#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

struct timespec start_time;

// Task function to simulate work
void perform_task()
{
    printf("Performing scheduled task...\n");
    // Example work: simulate small CPU load
    for (volatile long i = 0; i < 100000000; ++i)
        ;
}

// Signal handler for SIGALRM
void timer_handler(int signum)
{
    static int count = 0;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    double elapsed = (now.tv_sec - start_time.tv_sec) + (now.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("\n[%d] Timer fired! Elapsed time: %.6f seconds\n", ++count, elapsed);

    perform_task();
}

int main()
{
    struct sigaction sa;
    struct itimerval timer;

    // Get start time
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Setup signal handler
    sa.sa_handler = timer_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    // Configure timer (start after 2s, repeat every 2s)
    timer.it_value.tv_sec = 2;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 2;
    timer.it_interval.tv_usec = 0;

    // Start the periodic timer
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("setitimer failed");
        return 1;
    }

    printf("Scheduler started (PID: %d)\n", getpid());
    printf("Task runs every 2 seconds...\n");

    // Infinite loop waiting for timer events
    while (1)
    {
        pause(); // wait for next SIGALRM
    }

    return 0;
}
