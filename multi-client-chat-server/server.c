#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define SERVER_PORT 8080
#define MAX_USERS 10
#define MSG_BUF_SIZE 1024

int main(void)
{
    int listen_fd, new_fd;
    int client_fd[MAX_USERS];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    fd_set active_fds;
    int max_fd, ret_val;
    char msg_buf[MSG_BUF_SIZE];
    int bytes_read;

    /* initialize all client file descriptors */
    for (int i = 0; i < MAX_USERS; i++)
        client_fd[i] = -1;

    /* create TCP socket */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* enable port reuse */
    int opt_val = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) < 0)
    {
        perror("setsockopt()");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* setup server address */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);

    /* bind socket */
    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind()");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* start listening */
    if (listen(listen_fd, 5) < 0)
    {
        perror("listen()");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Chat server started on port %d...\n", SERVER_PORT);

    /* main server loop */
    while (1)
    {
        FD_ZERO(&active_fds);
        FD_SET(listen_fd, &active_fds);
        max_fd = listen_fd;

        /* add all connected clients */
        for (int i = 0; i < MAX_USERS; i++)
        {
            int fd = client_fd[i];
            if (fd > 0)
                FD_SET(fd, &active_fds);
            if (fd > max_fd)
                max_fd = fd;
        }

        /* wait for activity */
        ret_val = select(max_fd + 1, &active_fds, NULL, NULL, NULL);
        if (ret_val < 0 && errno != EINTR)
        {
            perror("select()");
            break;
        }

        /* check for new connections */
        if (FD_ISSET(listen_fd, &active_fds))
        {
            new_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);
            if (new_fd < 0)
            {
                perror("accept()");
                continue;
            }

            printf("New client connected: FD=%d, IP=%s, PORT=%d\n",
                   new_fd, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

            /* welcome message */
            const char *greet = "Hello! Welcome to the chat room.\n";
            send(new_fd, greet, strlen(greet), 0);

            /* store client socket */
            for (int i = 0; i < MAX_USERS; i++)
            {
                if (client_fd[i] == -1)
                {
                    client_fd[i] = new_fd;
                    printf("Assigned as client #%d\n", i + 1);
                    break;
                }
            }
        }

        /* handle data from existing clients */
        for (int i = 0; i < MAX_USERS; i++)
        {
            int fd = client_fd[i];

            if (fd > 0 && FD_ISSET(fd, &active_fds))
            {
                bytes_read = read(fd, msg_buf, sizeof(msg_buf) - 1);
                if (bytes_read <= 0)
                {
                    /* client disconnected */
                    getpeername(fd, (struct sockaddr *)&cli_addr, &cli_len);
                    printf("Client left: IP=%s, PORT=%d\n",
                           inet_ntoa(cli_addr.sin_addr),
                           ntohs(cli_addr.sin_port));
                    close(fd);
                    client_fd[i] = -1;
                }
                else
                {
                    msg_buf[bytes_read] = '\0';
                    printf("From client %d: %s", i + 1, msg_buf);

                    /* broadcast message to other clients */
                    for (int j = 0; j < MAX_USERS; j++)
                    {
                        if (client_fd[j] != -1 && client_fd[j] != fd)
                        {
                            send(client_fd[j], msg_buf, strlen(msg_buf), 0);
                        }
                    }
                }
            }
        }
    }

    close(listen_fd);
    return 0;
}
