#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<inttypes.h>
#include<stdbool.h>
#include<stdint.h>
#include<string.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

typedef struct {
    size_t os_page_size;
    size_t file_size;
}MmapConfig;

static void fatal(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static size_t get_os_page_size(void)
{
    long page_size = sysconf(_SC_PAGE_SIZE);
    if(page_size < 0)
    {
        return 4096;
    }
    return (size_t)page_size;
}


static uint8_t *map_file_shared(int fd, size_t length)
{
    void *address = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);

    if(address == MAP_FAILED)
    {
        fatal("mmap(MAP_SHARED)");
    }

    return (uint8_t*)address;
}

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s <file> <keyword>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *path = argv[1];
    const char *keyword = argv[2];
    size_t keyword_len = strlen(keyword);

    if(keyword_len == 0)
    {
        fprintf(stderr, "Keyword must not be empty.\n");
        exit(EXIT_FAILURE);
    }

    MmapConfig cfg = {0};
    cfg.os_page_size = get_os_page_size();

    int fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        fatal("open");
    }

    struct stat st;
    if(fstat(fd, &st) != 0)
    {
        fatal("fstat");
    }
    cfg.file_size = (size_t)st.st_size;

    printf("OS Page Size : %zu bytes\n", cfg.os_page_size);
    printf("File : %s (%zu bytes)\n", path, cfg.file_size);

    if(cfg.file_size == 0)
    {
        fprintf(stderr, "Empty file.\n");
        close(fd);
        return 0;
    }

    uint8_t *base = map_file_shared(fd, cfg.file_size);

    // Search for keyword
    size_t count = 0;
    for(size_t i = 0; i + keyword_len <= cfg.file_size; ++i)
    {
        if(memcmp(base + i, keyword, keyword_len) == 0)
        {
            printf("Found occurrence at offset %zu\n", i);
            count++;
        }
    }

    printf("\nTotal occurrences of \"%s\" = %zu\n", keyword, count);

    if(munmap(base, cfg.file_size) != 0)
    {
        fatal("munmap(shared)");
    }

    if(close(fd) != 0)
    {
        fatal("close");
    }

    return 0;
}

