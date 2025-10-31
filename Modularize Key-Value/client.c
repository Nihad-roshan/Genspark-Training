#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> 
#include <unistd.h>

#define SOCKET_PATH "/tmp/kvstore.sock"
#define BUF_SIZE 256

static void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void)
{
    int fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    char cmd[BUF_SIZE];

    // Create socket
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
        die("socket");

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        die("connect");

    printf("Connected to KV Store server at %s\n", SOCKET_PATH);
    printf("Type commands (SET key value / GET key / EXIT)\n\n");

    while (1)
    {
        printf("> ");
        fflush(stdout);

     
        if (fgets(cmd, sizeof(cmd), stdin) == NULL)//from reader
            break; // EOF or error


        cmd[strcspn(cmd, "\n")] = '\0';        // Remove trailing newline

        
        if (strcasecmp(cmd, "EXIT") == 0)
        {// Exit condition
            printf("Closing connection.\n");
            break;
        }

        // Send command to server
        ssize_t w = write(fd, cmd, strlen(cmd));
        if (w == -1)
        {
            perror("write");
            break;
        }
        else if (w != (ssize_t)strlen(cmd))
        {
            fprintf(stderr, "Partial write occurred\n");
            break;
        }

        // Read server response
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = '\0';
            printf("[server] %s", buf);
            if (buf[n - 1] != '\n') // Add newline if server didn't send one
                printf("\n");
        }
        else if (n == 0)
        {
            printf("[server closed connection]\n");
            break;
        }
        else
        {
            perror("read");
            break;
        }
    }

    close(fd);
    return 0;
}
