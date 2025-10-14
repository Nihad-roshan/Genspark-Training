#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Connect to localhost

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(1);
    }

    char buffer[BUF_SIZE];
    printf("Enter a message to send: ");
    fgets(buffer, BUF_SIZE, stdin);

    int n = write(sock, buffer, strlen(buffer));
    if (n < 0)
    {
        perror("Write failed");
        close(sock);
        exit(1);
    }

    n = read(sock, buffer, BUF_SIZE);
    if (n < 0)
    {
        perror("Read failed");
        close(sock);
        exit(1);
    }

    buffer[n] = '\0';
    printf("Server echoed: %s\n", buffer);

    close(sock);
    return 0;
}
