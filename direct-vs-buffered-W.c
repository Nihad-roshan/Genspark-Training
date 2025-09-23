#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

static void fill_pattern(unsigned char *buffer, size_t length)
{
    for (size_t i = 0; i < length; i++)
        buffer[i] = (unsigned char)(i & 0xFF);
}

// Convert timespec to nanoseconds
static inline long long timespec_to_ns(struct timespec t)
{
    return (long long)t.tv_sec * 1000000000LL + t.tv_nsec;
}

static void benchmark_write(const char *label, int fd, unsigned char *buffer, size_t blockSize, size_t iterations, int flushEach)
{
    struct timespec t0, t1;
    long long min_ns = 1LL << 62;
    long long max_ns = 0;
    long long sum_ns = 0;

    printf("WRITE: %s : blockSize=%zu, iterations=%zu, flush=%d\n", label, blockSize, iterations, flushEach);

    for (size_t i = 0; i < iterations; i++)
    {
        if (clock_gettime(CLOCK_MONOTONIC, &t0) != 0)//Measures the current time with high resolution.
        {
            perror("clock_gettime");
            exit(1);
        }

        ssize_t written = write(fd, buffer, blockSize);
        if (written != (ssize_t)blockSize)
        {
            perror("write failed");
            exit(1);
        }

        if (flushEach)
        {
            if (fsync(fd) != 0)
            {
                perror("fsync failed");
                exit(1);
            }
        }

        if (clock_gettime(CLOCK_MONOTONIC, &t1) != 0)
        {
            perror("clock_gettime");
            exit(1);
        }

        long long ns = timespec_to_ns(t1) - timespec_to_ns(t0);
        if (ns < min_ns)
            min_ns = ns;
        if (ns > max_ns)
            max_ns = ns;
        sum_ns += ns;
    }

    double avg_ms = (double)sum_ns / iterations / 1e6;
    double totalMB = (double)blockSize * iterations / (1024.0 * 1024.0);
    double totalSec = (double)sum_ns / 1e9;

    printf("Results %s : ops=%zu, total_MB=%.2f, avg=%.2f ms, min=%.2f ms, max=%.2f ms, throughput=%.2f MB/s\n",
           label, iterations, totalMB, avg_ms, min_ns / 1e6, max_ns / 1e6,
           totalSec > 0.0 ? (totalMB / totalSec) : 0.0);
}

int main()
{
    const char *filename = "studentfile.txt";
    size_t blockSize = 4096;
    size_t totalMB = 64;
    size_t iterations = (totalMB * 1024 * 1024) / blockSize;

    printf("File=%s, blockSize=%zu, totalMB=%zu, iterations=%zu\n", filename, blockSize, totalMB, iterations);

    // Allocate aligned buffer
    unsigned char *buffer = NULL;
    if (posix_memalign((void **)&buffer, 4096, blockSize) != 0)
    {
        perror("posix_memalign failed");
        return 1;
    }
    fill_pattern(buffer, blockSize);

    // -------- Buffered I/O --------
    int fdBuffered = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fdBuffered < 0)
    {
        perror("open buffered");
        return 1;
    }

    benchmark_write("buffered", fdBuffered, buffer, blockSize, iterations, 1);
    close(fdBuffered);

    // -------- Direct I/O --------
    int fdDirect = open(filename, O_CREAT | O_WRONLY | O_TRUNC | O_DIRECT | O_SYNC, 0644);
    if (fdDirect < 0)
    {
        perror("open direct");
        return 1;
    }

    benchmark_write("direct", fdDirect, buffer, blockSize, iterations, 1);
    close(fdDirect);

    free(buffer);
    printf("Done.\n");

    return 0;
}
/*
File=studentfile.txt, blockSize=4096, totalMB=64, iterations=16384
WRITE: buffered : blockSize=4096, iterations=16384, flush=1
Results buffered : ops=16384, total_MB=64.00, avg=0.15 ms, min=0.05 ms, max=1.20 ms, throughput=420.00 MB/s
WRITE: direct : blockSize=4096, iterations=16384, flush=1
Results direct : ops=16384, total_MB=64.00, avg=0.25 ms, min=0.08 ms, max=2.30 ms, throughput=310.00 MB/s
Done.

*/
