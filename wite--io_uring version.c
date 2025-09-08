#include <liburing.h> // io_uring API
#include <fcntl.h>    // open(), O_CREAT, O_WRONLY flags
#include <unistd.h>   // close(), write()
#include <string.h>   // strerror()
#include <stdio.h>    // printf(), perror()
#include <stdlib.h>   // aligned_alloc(), free()
#include <time.h>     // clock_gettime()
#include <errno.h>    // errno codes

// Helper: convert struct timespec to nanoseconds
static inline long long ts_ns(const struct timespec *t)
{
    return (long long)t->tv_sec * 1000000000LL + t->tv_nsec;
}

int main()
{
    const char *filename = "io_uring_write_test.bin"; // file to write
    size_t write_size = 4096;                         // each write block = 4 KB
    size_t total_mb = 8;                              // total MB to write

    // Calculate number of iterations = how many write operations
    size_t iterations = (total_mb * 1024 * 1024) / write_size;
    if (iterations == 0)
    {
        fprintf(stderr, "increase total_mb or reduce write_size\n");
        return 1;
    }

    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    unsigned char *buffer = aligned_alloc(4096, write_size);
    if (!buffer)
    {
        perror("aligned_alloc");
        return 1;
    }

    // Fill buffer with a simple repeating pattern
    for (size_t i = 0; i < write_size; i++)
    {
        buffer[i] = (unsigned char)(i & 0xFF);
    }

    struct io_uring ring;
    if (io_uring_queue_init(8, &ring, 0) < 0)
    {
        perror("io_uring_queue_init");
        return 1;
    }

    // -------------------------
    // Variables for timing stats
    // -------------------------
    struct timespec t0, t1;         // timestamps before/after I/O
    long long sum_ns = 0;           // accumulate total time
    long long min_ns = (1LL << 62); // start with huge number
    long long max_ns = 0;           // start with small number

    for (size_t i = 0; i < iterations; i++)
    {
        // File offset = block index * block size
        off_t offset = i * (off_t)write_size;

        // Get an SQE (Submission Queue Entry) to describe our request
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        if (!sqe)
        {
            fprintf(stderr, "io_uring_get_sqe returned NULL\n");
            return 1;
        }

        io_uring_prep_write(sqe, fd, buffer, write_size, offset);

        // Take timestamp before submission (start time)
        if (clock_gettime(CLOCK_MONOTONIC, &t0) < 0)
        {
            perror("clock_gettime");
            return 1;
        }

        io_uring_submit(&ring);
        
        // Wait for completion
        struct io_uring_cqe *cqe;
        int ret = io_uring_wait_cqe(&ring, &cqe);
        if (ret < 0)
        {
            fprintf(stderr, "io_uring_wait_cqe: %s\n", strerror(-ret));
            return 1;
        }


        // Validate result
        if (cqe->res < 0)
        {
            // cqe->res < 0 means an error occurred
            fprintf(stderr, "cqe->res error: %s\n", strerror(-cqe->res));
            return 1;
        }
        if ((size_t)cqe->res != write_size)
        {
            // Partial write (very rare here)
            fprintf(stderr, "short write: %d bytes\n", cqe->res);
            return 1;
        }

        // Take timestamp after completion (end time)
        if (clock_gettime(CLOCK_MONOTONIC, &t1) < 0)
        {
            perror("clock_gettime");
            return 1;
        }

        // Mark CQE as seen so io_uring can reuse it
        io_uring_cqe_seen(&ring, cqe);

        // Measure latency

        long long ns = ts_ns(&t1) - ts_ns(&t0);
        sum_ns += ns;
        if (ns < min_ns)
            min_ns = ns;
        if (ns > max_ns)
            max_ns = ns;
    }

    double avg_ms = (double)sum_ns / iterations / 1e6;
    printf("Write performance:\n");
    printf("  ops=%zu, avg=%.3f ms, min=%.3f ms, max=%.3f ms\n",
           iterations, avg_ms, min_ns / 1e6, max_ns / 1e6);


    io_uring_queue_exit(&ring); // free ring buffers
    close(fd);                  // close file
    free(buffer);               // free buffer memory

    return 0;
}

