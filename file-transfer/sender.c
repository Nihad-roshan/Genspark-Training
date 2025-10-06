// sender.c (Process A)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX 1024

int main()
{
    int write_fd = open("fifo1", O_WRONLY);
    int read_fd = open("fifo2", O_RDONLY);
    if (write_fd < 0 || read_fd < 0)
    {
        perror("open");
        exit(1);
    }

    char filename[100];
    printf("Enter file to send: ");
    scanf("%s", filename);

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        perror("fopen");
        exit(1);
    }

    char buffer[MAX];
    size_t bytes;
    while ((bytes = fread(buffer, 1, MAX, fp)) > 0)
    {
        write(write_fd, buffer, bytes);
    }

    // Indicate end of transfer
    write(write_fd, "EOF", 4);

    // Wait for acknowledgment
    read(read_fd, buffer, MAX);
    printf("Receiver says: %s\n", buffer);

    fclose(fp);
    close(write_fd);
    close(read_fd);
    return 0;
}
