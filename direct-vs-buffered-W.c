#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h> // <-- needed for _aligned_malloc / _aligned_free



static void fill_pattern(unsigned char *buffer, size_t length)
{ // Fills the buffer with a repeating pattern 0,1,2,...255,0,1,....
    for (size_t i = 0; i < length; i++)
        buffer[i] = (unsigned char)(i & 0xFF);
}

static void benchmark_write(const char *label, HANDLE hFile, unsigned char *buffer, size_t blockSize, size_t iterations, int flushEach)
{
    /*
    label: “buffered” or “direct” (for display).

hFile: Windows file handle (already opened).

buffer: pointer to aligned memory buffer.

blockSize: number of bytes written per operation.

iterations: number of times to write.

flushEach: whether to call FlushFileBuffers() after each write.
*/
    LARGE_INTEGER freq, t0, t1;
    QueryPerformanceFrequency(&freq); // Gets the frequency of the performance counter (ticks per second).

    // Tracks minimum, maximum, and sum of latencies (in nanoseconds).
    long long min_ns = (1LL << 62);
    long long max_ns = 0;
    long long sum_ns = 0;

    DWORD bytesWritten; // bytesWritten: for WriteFile

    // Printing benchmark parameters.
    printf("WRITE: %s : blockSize=%zu, iterations=%zu, flush=%d\n", label, blockSize, iterations, flushEach);

    for (size_t i = 0; i < iterations; i++)
    {
        QueryPerformanceCounter(&t0); // start timing

        BOOL ok = WriteFile(hFile, buffer, (DWORD)blockSize, &bytesWritten, NULL); // Call WriteFile() to write one block.
        if (!ok || bytesWritten != blockSize)
        {
            printf("WriteFile failed (%lu)\n", GetLastError());
            return;
        }

        if (flushEach)
        {
            FlushFileBuffers(hFile);
        }

        QueryPerformanceCounter(&t1); // End timing.

        long long ns = timespec_to_ns(t1, freq) - timespec_to_ns(t0, freq);

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
    size_t blockSize = 4096; // must be multiple of sector size (usually 4096)
    size_t totalMB = 64;
    size_t iterations = (totalMB * 1024 * 1024) / blockSize;

    printf("File=%s, blockSize=%zu, totalMB=%zu, iterations=%zu\n", filename, blockSize, totalMB, iterations);

    // Allocate aligned buffer
    unsigned char *buffer = (unsigned char *)_aligned_malloc(blockSize, 4096);
    if (!buffer)
    {
        printf("aligned malloc failed\n");
        return 1;
    }
    fill_pattern(buffer, blockSize);

    // -------- Buffered I/O --------
    HANDLE hBuffered = CreateFileA(filename,
                                   GENERIC_WRITE,
                                   0, NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL, NULL);

    if (hBuffered == INVALID_HANDLE_VALUE)
    {
        printf("Open buffered failed (%lu)\n", GetLastError());
        return 1;
    }

    benchmark_write("buffered", hBuffered, buffer, blockSize, iterations, 1);
    CloseHandle(hBuffered);

    // -------- Direct I/O --------
    HANDLE hDirect = CreateFileA(filename,
                                 GENERIC_WRITE,
                                 0, NULL,
                                 CREATE_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
                                 NULL);

    if (hDirect == INVALID_HANDLE_VALUE)
    {
        printf("Open direct failed (%lu)\n", GetLastError());
        return 1;
    }

    benchmark_write("direct", hDirect, buffer, blockSize, iterations, 1);
    CloseHandle(hDirect);

    _aligned_free(buffer);
    printf("Done.\n");

    return 0;
}
