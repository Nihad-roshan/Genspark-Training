#define _GNU_SOURCE
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define QUEUE_DEPTH 4   // io_uring submission queue depth
#define BLOCK_SIZE 4096 // block size (alignment for buffers)

typedef struct
{
    int input_fd;     // file descriptor of input file
    off_t offset;     // read offset (unused since we read full file at once)
    size_t file_size; // total size of the input file
    char *buffer;     // buffer holding file data
    char *file_name;  // input file name (for error/debug messages)
} ReadRequest;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s output_file input1 input2 ...\n", argv[0]);
        return 1;
    }

    const char *output_file_name = argv[1];

    // open output file for writing (truncate if exists, create if not)
    int output_fd = open(output_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd < 0)
    {
        perror("open output file");
        return 1;
    }

    // initialize io_uring with given queue depth
    struct io_uring ring;
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0)
    {
        perror("io_uring_queue_init");
        return 1;
    }

    // submit async reads for all input files
    for (int file_index = 2; file_index < argc; file_index++)
    {
        int input_fd = open(argv[file_index], O_RDONLY);
        if (input_fd < 0)
        {
            perror("open input file");
            continue; // skip this file
        }

        struct stat file_stat;
        if (fstat(input_fd, &file_stat) < 0)
        {
            perror("fstat");
            close(input_fd);
            continue;
        }

        size_t file_size = file_stat.st_size;

        // allocate aligned buffer to hold the entire file
        char *file_buffer = aligned_alloc(BLOCK_SIZE, file_size);
        if (!file_buffer)
        {
            perror("aligned_alloc");
            exit(1);
        }

        // get a submission queue entry
        struct io_uring_sqe *submission_entry = io_uring_get_sqe(&ring);
        if (!submission_entry)
        {
            fprintf(stderr, "Failed to get submission queue entry\n");
            exit(1);
        }

        // allocate and fill request structure
        ReadRequest *request = malloc(sizeof(*request));
        request->input_fd = input_fd;
        request->offset = 0;
        request->file_size = file_size;
        request->buffer = file_buffer;
        request->file_name = argv[file_index];

        // prepare async read operation
        io_uring_prep_read(submission_entry, input_fd, file_buffer, file_size, 0);

        // attach our request struct as user data
        io_uring_sqe_set_data(submission_entry, request);
    }

    // submit all queued read requests
    io_uring_submit(&ring);

    // process completions in the order they arrive
    for (int completed_files = 2; completed_files < argc; completed_files++)
    {
        struct io_uring_cqe *completion_entry;
        io_uring_wait_cqe(&ring, &completion_entry);

        // retrieve our request data
        ReadRequest *request = io_uring_cqe_get_data(completion_entry);

        if (completion_entry->res < 0)
        {
            fprintf(stderr, "Error reading %s: %s\n",
                    request->file_name, strerror(-completion_entry->res));
        }
        else
        {
            // merge: write file contents into output in order of completion
            ssize_t bytes_written = write(output_fd, request->buffer, request->file_size);
            if (bytes_written != request->file_size)
            {
                perror("write to output file");
            }
            else
            {
                printf("Merged file: %s (%ld bytes)\n",
                       request->file_name, request->file_size);
            }
        }

        // mark this completion as seen
        io_uring_cqe_seen(&ring, completion_entry);

        // cleanup
        close(request->input_fd);
        free(request->buffer);
        free(request);
    }

    // cleanup io_uring and output file
    io_uring_queue_exit(&ring);
    close(output_fd);

    return 0;
}
