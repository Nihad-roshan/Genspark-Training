#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9000
#define MAX_CLIENTS FD_SETSIZE  //1024 fds
#define BUF_SIZE 1024


int set_nonblocking(int fd)
{//making a socket non-blocking
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(void)
{
    int listen_fd, conn_fd, max_fd, activity;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    char buffer[BUF_SIZE];
    int clients[MAX_CLIENTS]; // Track connected clients

    // Create listening socket
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    // Allow reuse of address
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //It tells the kernel to allow your server to reuse the same port immediately after restarting, without waiting for the old connection to time out.

    // Non-blocking socket
    set_nonblocking(listen_fd);

    // Bind to all interfaces on PORT
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; //It tells the server to accept connections on any network interface (any IP address) available on the host.
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 10) < 0)
    {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Listening on port %d...\n", PORT);

    // Initialize client tracking array
    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i] = -1;

    fd_set readfds, writefds;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        FD_SET(listen_fd, &readfds);
        max_fd = listen_fd;

        // Add client fds to readfds
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];
            if (fd > 0)
            {
                FD_SET(fd, &readfds);
                if (fd > max_fd)
                    max_fd = fd;
            }
        }

        // Wait for activity
        activity = select(max_fd + 1, &readfds, &writefds, NULL, NULL);

        if (activity < 0 && errno != EINTR)
        {
            perror("select()");
            break;
        }

        // New connection
        if (FD_ISSET(listen_fd, &readfds)) ///This checks whether the listening socket (listen_fd) is ready to read
        {
            conn_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
            if (conn_fd < 0)
            {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                    perror("accept()");
            }
            else
            {
                set_nonblocking(conn_fd);
                printf("[SERVER] New client connected (fd=%d)\n", conn_fd);

                // Add to client list
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients[i] == -1)
                    {
                        clients[i] = conn_fd;
                        break;
                    }
                }
            }
        }

        // Handle I/O for each client
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];
            if (fd < 0)
                continue;

            if (FD_ISSET(fd, &readfds)) //This checks whether the listening socket (listen_fd) is ready to read
            {
                ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
                if (n > 0)
                {
                    buffer[n] = '\0';
                    printf("[SERVER] Received from fd=%d: %s", fd, buffer);
                    // Echo back
                    write(fd, buffer, n);
                }
                else if (n == 0)
                {
                    printf("[SERVER] Client fd=%d disconnected\n", fd);
                    close(fd);
                    clients[i] = -1;
                }
                else if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    perror("read()");
                    close(fd);
                    clients[i] = -1;
                }
            }
        }
    }

    close(listen_fd);
    return 0;
}


/*
 * This server:
 *  - Accepts multiple clients simultaneously.
 *  - Uses select() to multiplex sockets.
 *  - Handles non-blocking sockets for efficient I/O.
 */