// receiver.c (Process B)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX 1024

int main()
{
    int read_fd = open("fifo1", O_RDONLY);
    int write_fd = open("fifo2", O_WRONLY);
    if (write_fd < 0 || read_fd < 0)
    {
        perror("open");
        exit(1);
    }

    FILE *fp = fopen("received_file.txt", "wb");
    if (!fp)
    {
        perror("fopen");
        exit(1);
    }

    char buffer[MAX];
    ssize_t n;
    while ((n = read(read_fd, buffer, MAX)) > 0)
    {
        if (strncmp(buffer, "EOF", 3) == 0)
            break;
        fwrite(buffer, 1, n, fp);
    }

    printf("File received successfully.\n");
    write(write_fd, "Transfer complete", 18);

    fclose(fp);
    close(write_fd);
    close(read_fd);
    return 0;
}
