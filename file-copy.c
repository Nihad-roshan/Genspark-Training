#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

ssize_t read_all(int fd, void *buf, size_t count) 
{
    char *p = buf;
    size_t left = count;

    while (left > 0) 
    {
        ssize_t n = read(fd, p, left);
        if (n < 0) 
        {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) break; // EOF
        p += n;
        left -= n;
    }
    return count - left;
}

ssize_t write_all(int fd, const void *buf, size_t count) 
{
    const char *p = buf;
    size_t left = count;

    while (left > 0) 
    {
        ssize_t n = write(fd, p, left);
        if (n < 0) 
        {
            if (errno == EINTR) continue;
            return -1;
        }
        p += n;
        left -= n;
    }
    return count;
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        printf("Usage: %s <source> <dest>\n", argv[0]);
        return 1;
    }

    int src_fd = open(argv[1], O_RDONLY);
    if (src_fd < 0) 
    {
        perror("open src");
        return 1;
    }

    int dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd < 0) 
    {
        perror("open dst");
        close(src_fd);
        return 1;
    }

    char buf[4096];
    ssize_t n;

    // ---- File Copy ----
    while ((n = read_all(src_fd, buf, sizeof(buf))) > 0) 
    {
        if (write_all(dst_fd, buf, n) != n) 
        {
            perror("write");
            close(src_fd);
            close(dst_fd);
            return 1;
        }
    }

    if (n < 0) 
    {
        perror("read");
        close(src_fd);
        close(dst_fd);
        return 1;
    }

    // ---- Verification using lseek ----
    off_t src_size = lseek(src_fd, 0, SEEK_END);
    off_t dst_size = lseek(dst_fd, 0, SEEK_END);

    if (src_size != dst_size) 
    {
        printf("Size mismatch: source=%ld bytes, dest=%ld bytes\n",
               (long)src_size, (long)dst_size);
    } 
    else 
    {
        // Reset offsets
        lseek(src_fd, 0, SEEK_SET);
        lseek(dst_fd, 0, SEEK_SET);

        char c1, c2;
        off_t pos = 0;
        int mismatch = 0;

        while (read(src_fd, &c1, 1) == 1 && read(dst_fd, &c2, 1) == 1) 
        {
            if (c1 != c2) 
            {
                printf("Mismatch at byte %ld: src=0x%02x, dest=0x%02x\n",
                       (long)pos, (unsigned char)c1, (unsigned char)c2);
                mismatch = 1;
                break;  // stop at first mismatch
            }
            pos++;
        }

        if (!mismatch) 
        {
            printf("File copied successfully and verified: identical content.\n");
        }
    }

    close(src_fd);
    close(dst_fd);

    return 0;
}
