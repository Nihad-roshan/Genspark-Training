#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    for (int i = 0; i < 5; i++)
    {
        int fd = open("demo.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0)
        {
            return 1;
        }
        write(fd,"Hello",6);
        close(fd);
    }
    return 0;
}
