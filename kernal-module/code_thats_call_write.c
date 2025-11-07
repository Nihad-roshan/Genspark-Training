// trigger.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int fd = open("/proc/syslog_demo", O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    const char *msg = "hello from user space";
    ssize_t n = write(fd, msg, strlen(msg));
    if (n < 0)
    {
        perror("write");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
