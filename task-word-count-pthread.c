#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>

#define WORKER_COUNT 4

typedef struct
{
    char *data;
    size_t start;
    size_t end;
    long count;
} worker_arg_t;

static long global_word_count = 0;
static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

// Worker thread function
void *worker_thread(void *arg)
{
    worker_arg_t *warg = (worker_arg_t *)arg;
    int in_word = 0;
    long local_count = 0;

    for (size_t i = warg->start; i < warg->end; i++)
    {
        if (isspace((unsigned char)warg->data[i]))//isspace() checks if the current character is a space, tab, newline, etc.
        {
            if (in_word)
            {
                local_count++;
                in_word = 0;
            }
        }
        else
        {
            in_word = 1;
        }
    }
    if (in_word)
        local_count++;

    // Update global counter
    pthread_mutex_lock(&count_mutex);
    global_word_count += local_count;
    pthread_mutex_unlock(&count_mutex);

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    // Read whole file into memory
    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
    {
        perror("fopen");
        exit(1);
    }

    struct stat st;
    if (stat(argv[1], &st) != 0)
    {//stat() = “go ask the OS how big this file is and other details.
        perror("stat");
        exit(1);
    }

    size_t filesize = st.st_size;
    char *data = malloc(filesize);
    if (!data)
    {
        perror("malloc");
        exit(1);
    }

    if (fread(data, 1, filesize, fp) != filesize)
    {
        perror("fread");
        exit(1);
    }
    fclose(fp);

    pthread_t workers[WORKER_COUNT];
    worker_arg_t args[WORKER_COUNT];

    // Divide file into chunks
    size_t chunk_size = filesize / WORKER_COUNT;
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        args[i].data = data;
        args[i].start = i * chunk_size;
        args[i].end = (i == WORKER_COUNT - 1) ? filesize : (i + 1) * chunk_size;
        args[i].count = 0;

        pthread_create(&workers[i], NULL, worker_thread, &args[i]);
    }

    // Wait for all workers
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        pthread_join(workers[i], NULL);
    }

    printf("Total Word Count: %ld\n", global_word_count);

    free(data);
    pthread_mutex_destroy(&count_mutex);
    return 0;
}

