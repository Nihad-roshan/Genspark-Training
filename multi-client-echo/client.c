#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUF_SIZE 1024

int main(void)
{
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) //inet_pton() is used to convert an IP address from human-readable text format (like "127.0.0.1") into binary form that can be used in socket programming.
    {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    printf("[CLIENT] Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);
    printf("Type messages (Ctrl+C to exit):\n");

    while (1)
    {
        printf("> ");
        fflush(stdout);

        if (!fgets(buffer, sizeof(buffer), stdin))
            break; // EOF

        if (write(sock, buffer, strlen(buffer)) < 0)
        {
            perror("write()");
            break;
        }

        ssize_t n = read(sock, buffer, sizeof(buffer) - 1);
        if (n <= 0)
        {
            perror("read()");
            break;
        }

        buffer[n] = '\0';
        printf("[SERVER ECHO]: %s", buffer);
    }

    close(sock);
    return 0;
}
