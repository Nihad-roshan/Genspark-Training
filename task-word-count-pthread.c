#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define QUEUE_SIZE 8
#define WORKER_COUNT 4
#define CHUNK_SIZE 1024 // 1KB chunks

typedef struct
{
    int id;
    char *chunk;
    size_t length;
} request_t;

static request_t queue[QUEUE_SIZE];
static int q_head = 0;
static int q_tail = 0;
static int q_count = 0;

static pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t q_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t q_not_full = PTHREAD_COND_INITIALIZER;

static int next_request_id = 1;
static pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;

// Global word count
static long global_word_count = 0;
static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

// -------- Queue Operations ----------
static void enqueue(request_t request)
{
    pthread_mutex_lock(&q_mutex);

    while (q_count == QUEUE_SIZE)
    {
        pthread_cond_wait(&q_not_full, &q_mutex);
    }

    queue[q_tail] = request;
    q_tail = (q_tail + 1) % QUEUE_SIZE;
    q_count++;

    pthread_cond_signal(&q_not_empty);
    pthread_mutex_unlock(&q_mutex);
}

static request_t dequeue(void)
{
    pthread_mutex_lock(&q_mutex);

    while (q_count == 0)
    {
        pthread_cond_wait(&q_not_empty, &q_mutex);
    }

    request_t request = queue[q_head];
    q_head = (q_head + 1) % QUEUE_SIZE;
    q_count--;

    pthread_cond_signal(&q_not_full);
    pthread_mutex_unlock(&q_mutex);

    return request;
}

// -------- Worker Thread (Consumers) ----------
static void *worker_thread(void *arg)
{
    int worker_id = *(int *)arg;

    while (1)
    {
        request_t req = dequeue();
        if (req.id == -1)
        { // shutdown signal
            break;
        }

        // Count words in the chunk
        int in_word = 0;
        long local_count = 0;
        for (size_t i = 0; i < req.length; i++)
        {
            if (isspace((unsigned char)req.chunk[i]))
            {
                if (in_word)
                {
                    in_word = 0;
                    local_count++;
                }
            }
            else
            {
                in_word = 1;
            }
        }
        if (in_word)
            local_count++;

        // Update global counter safely
        pthread_mutex_lock(&count_mutex);
        global_word_count += local_count;
        pthread_mutex_unlock(&count_mutex);

        free(req.chunk);
    }
    return NULL;
}

// -------- File Splitter (Producer) ----------
static void split_file_into_chunks(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        perror("fopen");
        exit(1);
    }

    char *buffer = malloc(CHUNK_SIZE);
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, fp)) > 0)
    {
        request_t req;

        pthread_mutex_lock(&id_mutex);
        req.id = next_request_id++;
        pthread_mutex_unlock(&id_mutex);

        req.chunk = malloc(bytes_read);
        memcpy(req.chunk, buffer, bytes_read);
        req.length = bytes_read;

        enqueue(req);
    }

    free(buffer);
    fclose(fp);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    srand((unsigned)time(NULL));

    pthread_t workers[WORKER_COUNT];
    int worker_ids[WORKER_COUNT];

    // Start worker threads
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        worker_ids[i] = i + 1;
        pthread_create(&workers[i], NULL, worker_thread, &worker_ids[i]);
    }

    // Producer: split file into chunks
    split_file_into_chunks(argv[1]);

    // Send shutdown signals
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        request_t signal = {.id = -1, .chunk = NULL, .length = 0};
        enqueue(signal);
    }

    // Wait for workers
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        pthread_join(workers[i], NULL);
    }

    printf("Total Word Count: %ld\n", global_word_count);

    pthread_mutex_destroy(&q_mutex);
    pthread_mutex_destroy(&id_mutex);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&q_not_empty);
    pthread_cond_destroy(&q_not_full);

    return 0;
}
