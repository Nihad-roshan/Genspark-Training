#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/time.h>

#define PORT 8080
#define BUF_SIZE 1024

// Function to get current time in microseconds
long get_time_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

// Thread handler
void *thread_handler(void *arg)
{
    int client_sock = *(int *)arg;
    free(arg);
    char buffer[BUF_SIZE];

    long start = get_time_us();
    int n = read(client_sock, buffer, BUF_SIZE);
    buffer[n] = '\0';
    write(client_sock, buffer, n);
    long end = get_time_us();

    printf("[Thread] Request handled in %ld microseconds\n", end - start);
    close(client_sock);
    return NULL;
}

// Fork handler
void fork_handler(int client_sock)
{
    char buffer[BUF_SIZE];
    long start = get_time_us();
    int n = read(client_sock, buffer, BUF_SIZE);
    buffer[n] = '\0';
    write(client_sock, buffer, n);
    long end = get_time_us();

    printf("[Fork] Request handled in %ld microseconds\n", end - start);
    close(client_sock);
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <fork|thread>\n", argv[0]);
        return 1;
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Socket failed");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_sock, 10) < 0)
    {
        perror("Listen failed");
        exit(1);
    }

    printf("Server running on port %d in %s mode...\n", PORT, argv[1]);

    while (1)
    {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0)
        {
            perror("Accept failed");
            continue;
        }

        if (strcmp(argv[1], "fork") == 0)
        {
            pid_t pid = fork();
            if (pid == 0)
            { // child
                fork_handler(client_sock);
            }
            else
            {
                close(client_sock);         // parent closes the socket
                waitpid(-1, NULL, WNOHANG); // prevent zombie processes
            }
        }
        else if (strcmp(argv[1], "thread") == 0)
        {
            pthread_t tid;
            int *pclient = malloc(sizeof(int));
            *pclient = client_sock;
            pthread_create(&tid, NULL, thread_handler, pclient);
            pthread_detach(tid); // automatically reclaim thread resources
        }
        else
        {
            printf("Unknown mode: %s\n", argv[1]);
            close(client_sock);
        }
    }

    close(server_sock);
    return 0;
}


