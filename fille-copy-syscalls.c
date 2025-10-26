#include <fcntl.h>  // For open() flags
#include <unistd.h> // For read(), write(), close(), lseek()
#include <stdio.h>  // For perror()
#include <stdlib.h> // For exit()
#include <errno.h>  // For errno

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    int src_fd, dest_fd;
    ssize_t bytes_read, bytes_written, total_read = 0, total_written = 0;
    char buffer[BUF_SIZE];

    if (argc != 3)
    {
        write(STDERR_FILENO, "Usage: ./file_copy <source> <destination>\n", 42);
        exit(EXIT_FAILURE);
    }

    // Open source file (read-only)
    src_fd = open(argv[1], O_RDONLY);
    if (src_fd < 0)
    {
        perror("Error opening source file");
        exit(EXIT_FAILURE);
    }

    // Open destination file (create or truncate for writing)
    dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0)
    {
        perror("Error opening/creating destination file");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    // Copy loop
    while ((bytes_read = read(src_fd, buffer, BUF_SIZE)) > 0)
    {
        total_read += bytes_read;

        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written < 0)
        {
            perror("Error writing to destination file");
            close(src_fd);
            close(dest_fd);
            exit(EXIT_FAILURE);
        }

        total_written += bytes_written;
    }

    if (bytes_read < 0)
    {
        perror("Error reading source file");
        close(src_fd);
        close(dest_fd);
        exit(EXIT_FAILURE);
    }

    // Close files
    if (close(src_fd) < 0)
    {
        perror("Error closing source file");
        exit(EXIT_FAILURE);
    }
    if (close(dest_fd) < 0)
    {
        perror("Error closing destination file");
        exit(EXIT_FAILURE);
    }

    // Print total counts (using write)
    char msg[128];
    int len = snprintf(msg, sizeof(msg),
                       "Total bytes read: %zd\nTotal bytes written: %zd\n",
                       total_read, total_written);
    write(STDOUT_FILENO, msg, len);

    return 0;
}
