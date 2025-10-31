#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <dlfcn.h>
#include "kvstore.h"

#define SOCKET_PATH "/tmp/kvstore.sock"
#define BACKLOG 10
#define BUF_SIZE 256

typedef const char *(*kv_get_t)(const char *);
typedef void (*kv_set_t)(const char *, const char *);

static kv_get_t kv_get_func;
static kv_set_t kv_set_func;

void die(const char *msg)
{
    perror(msg);
    unlink(SOCKET_PATH);
    exit(EXIT_FAILURE);
}

void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buf[BUF_SIZE];
    while (1)
    {
        ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
        if (n <= 0)
            break;

        buf[n] = '\0';
        char key[BUF_SIZE], value[BUF_SIZE];

        if (sscanf(buf, "SET %255s %255[^\n]", key, value) == 2)
        {
            kv_set_func(key, value);
            if (write(client_fd, "OK\n", 3) < 0)
            {
                perror("write");
                break;
            }
        }
        else if (sscanf(buf, "GET %255s", key) == 1)
        {
            const char *val = kv_get_func(key);
            if (val)
                dprintf(client_fd, "%s\n", val);
            else if (write(client_fd, "NOT_FOUND\n", 10) < 0)
            {
                perror("write");
                break;
            }
        }
        else
        {
            if (write(client_fd, "ERROR\n", 6) < 0)
            {
                perror("write");
                break;
            }
        }
    }

    close(client_fd);
    return NULL;
}

int main(void)
{
    
    void *handle = dlopen("./libkvstore.so", RTLD_NOW);//// Load dynamic library
    if (!handle)
    {
        fprintf(stderr, "Error loading libkvstore.so: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    kv_get_func = (kv_get_t)dlsym(handle, "kv_get");
    kv_set_func = (kv_set_t)dlsym(handle, "kv_set");
    if (!kv_get_func || !kv_set_func)
    {
        fprintf(stderr, "Error loading symbols: %s\n", dlerror());
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd == -1)
        die("socket");

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(SOCKET_PATH);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        die("bind");
    if (listen(listen_fd, BACKLOG) == -1)
        die("listen");

    printf("Multithreaded server running with dlopen() on %s\n", SOCKET_PATH);

    while (1)
    {
        int *client_fd = malloc(sizeof(int));
        if (!client_fd)
            die("malloc");

        *client_fd = accept(listen_fd, NULL, NULL);
        if (*client_fd == -1)
        {
            free(client_fd);
            if (errno == EINTR)
                continue;
            die("accept");
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, client_fd) != 0)
        {
            perror("pthread_create");
            close(*client_fd);
            free(client_fd);
            continue;
        }
        pthread_detach(tid);
    }

    close(listen_fd);
    unlink(SOCKET_PATH);
    dlclose(handle);
    return 0;
}
