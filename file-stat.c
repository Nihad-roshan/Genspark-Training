#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    char path[512];
    long total_size = 0;

    if (argc != 2) {
        write(STDERR_FILENO, "Usage: ./dir_sysinfo <directory>\n", 33);
        exit(EXIT_FAILURE);
    }

    dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "File Name\tSize (bytes)\tPermissions\n", 36);
    write(STDOUT_FILENO, "--------------------------------------\n", 39);

    while ((entry = readdir(dir)) != NULL) {
        // Skip current and parent directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", argv[1], entry->d_name);

        if (stat(path, &fileStat) == -1) {
            perror("stat error");
            continue;
        }

        // Print file name
        write(STDOUT_FILENO, entry->d_name, strlen(entry->d_name));
        write(STDOUT_FILENO, "\t", 1);

        // Print file size
        char sizebuf[64];
        int len = snprintf(sizebuf, sizeof(sizebuf), "%ld\t", fileStat.st_size);
        write(STDOUT_FILENO, sizebuf, len);

        // Print permissions (user/group/other)
        char perms[10];
        perms[0] = (fileStat.st_mode & S_IRUSR) ? 'r' : '-';
        perms[1] = (fileStat.st_mode & S_IWUSR) ? 'w' : '-';
        perms[2] = (fileStat.st_mode & S_IXUSR) ? 'x' : '-';
        perms[3] = (fileStat.st_mode & S_IRGRP) ? 'r' : '-';
        perms[4] = (fileStat.st_mode & S_IWGRP) ? 'w' : '-';
        perms[5] = (fileStat.st_mode & S_IXGRP) ? 'x' : '-';
        perms[6] = (fileStat.st_mode & S_IROTH) ? 'r' : '-';
        perms[7] = (fileStat.st_mode & S_IWOTH) ? 'w' : '-';
        perms[8] = (fileStat.st_mode & S_IXOTH) ? 'x' : '-';
        perms[9] = '\0';
        write(STDOUT_FILENO, perms, strlen(perms));

        write(STDOUT_FILENO, "\n", 1);

        total_size += fileStat.st_size;
    }

    closedir(dir);

    // Print total size
    char totalmsg[128];
    int tlen = snprintf(totalmsg, sizeof(totalmsg),
                        "\nTotal size of all files: %ld bytes\n", total_size);
    write(STDOUT_FILENO, totalmsg, tlen);

    return 0;
}
