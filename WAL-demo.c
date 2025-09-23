#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define WAL_FILE "wal.log"
#define DB_FILE "db.txt"
#define MAX_KEY_LEN 256
#define MAX_VALUE_LEN 256
#define MAX_LINE_LEN 512

// Function prototypes
void write_wal_with_fsync(const char *key, const char *value);
void write_wal_without_fsync(const char *key, const char *value);
void crash_after_wal(const char *key, const char *value);
void recover_from_wal();
void display_wal_and_db();
void apply_transaction_to_db(const char *key, const char *value);

// Global transaction counter
int transaction_id = 1;

// Helper to fsync a FILE pointer
void fsync_file(FILE *file)
{
    fflush(file);
    fsync(fileno(file));
}

// Write key-value with WAL and fsync
void write_wal_with_fsync(const char *key, const char *value)
{
    FILE *wal_fp = fopen(WAL_FILE, "a");
    if (!wal_fp)
    {
        perror("Failed to open WAL file");
        return;
    }

    fprintf(wal_fp, "TRANSACTION %d BEGIN\n", transaction_id);
    fprintf(wal_fp, "SET %s %s\n", key, value);
    fprintf(wal_fp, "TRANSACTION %d COMMIT\n", transaction_id);

    fsync_file(wal_fp); // force write to disk
    fclose(wal_fp);

    apply_transaction_to_db(key, value); // apply to DB
    transaction_id++;
}

// Write key-value with WAL but no fsync
void write_wal_without_fsync(const char *key, const char *value)
{
    FILE *wal_fp = fopen(WAL_FILE, "a");
    if (!wal_fp)
    {
        perror("Failed to open WAL file");
        return;
    }

    fprintf(wal_fp, "TRANSACTION %d BEGIN\n", transaction_id);
    fprintf(wal_fp, "SET %s %s\n", key, value);
    fprintf(wal_fp, "TRANSACTION %d COMMIT\n", transaction_id);

    // no fsync here
    fclose(wal_fp);

    apply_transaction_to_db(key, value); // apply to DB
    transaction_id++;
}

// Simulate crash after WAL write
void crash_after_wal(const char *key, const char *value)
{
    FILE *wal_fp = fopen(WAL_FILE, "a");
    if (!wal_fp)
    {
        perror("Failed to open WAL file");
        return;
    }

    fprintf(wal_fp, "TRANSACTION %d BEGIN\n", transaction_id);
    fprintf(wal_fp, "SET %s %s\n", key, value);
    fprintf(wal_fp, "TRANSACTION %d COMMIT\n", transaction_id);

    fsync_file(wal_fp); // flush WAL to disk
    fclose(wal_fp);

    printf("Simulating crash BEFORE applying to DB\n");
    exit(1); // crash before updating db.txt
}

// Apply key-value to DB (db.txt)
void apply_transaction_to_db(const char *key, const char *value)
{
    FILE *db_fp = fopen(DB_FILE, "a");
    if (!db_fp)
    {
        perror("Failed to open DB file");
        return;
    }

    fprintf(db_fp, "%s %s\n", key, value);
    fclose(db_fp);
}

// Recover committed transactions from WAL
void recover_from_wal()
{
    FILE *wal_fp = fopen(WAL_FILE, "r");
    if (!wal_fp)
    {
        perror("Failed to open WAL for recovery");
        return;
    }

    FILE *db_fp = fopen(DB_FILE, "w"); // overwrite db.txt
    if (!db_fp)
    {
        perror("Failed to open DB file for recovery");
        fclose(wal_fp);
        return;
    }

    char line[MAX_LINE_LEN];
    char current_key[MAX_KEY_LEN];
    char current_value[MAX_VALUE_LEN];
    int in_transaction = 0;
    int transaction_committed = 0;

    while (fgets(line, sizeof(line), wal_fp))//Read WAL line by line.
    {
        if (sscanf(line, "TRANSACTION %*d BEGIN") == 0)//If the line is "TRANSACTION X BEGIN" → mark that a new transaction started. //%*d skips transaction ID.
        {
            in_transaction = 1;
            transaction_committed = 0;
        }
        else if (sscanf(line, "SET %s %s", current_key, current_value) == 2)
        {
            // store SET values temporarily
        }
        else if (sscanf(line, "TRANSACTION %*d COMMIT") == 0)
        {
            if (in_transaction)
            {
                fprintf(db_fp, "%s %s\n", current_key, current_value);
                in_transaction = 0;
            }
        }
    }

    fclose(wal_fp);
    fclose(db_fp);
}

// Display WAL and DB contents
void display_wal_and_db()
{
    printf("===== WAL LOG =====\n");
    FILE *wal_fp = fopen(WAL_FILE, "r");
    if (wal_fp)
    {
        char line[MAX_LINE_LEN];
        while (fgets(line, sizeof(line), wal_fp))
        {
            printf("%s", line);
        }
        fclose(wal_fp);
    }

    printf("\n===== DB CONTENTS =====\n");
    FILE *db_fp = fopen(DB_FILE, "r");
    if (db_fp)
    {
        char line[MAX_LINE_LEN];
        while (fgets(line, sizeof(line), db_fp))
        {
            printf("%s", line);
        }
        fclose(db_fp);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <command> [key] [value]\n", argv[0]);
        return 1;
    }

    char *command = argv[1];
    if (strcmp(command, "write") == 0 && argc == 4)
    {
        write_wal_with_fsync(argv[2], argv[3]);
    }
    else if (strcmp(command, "write-nosync") == 0 && argc == 4)
    {
        write_wal_without_fsync(argv[2], argv[3]);
    }
    else if (strcmp(command, "crash-after-wal") == 0 && argc == 4)
    {
        crash_after_wal(argv[2], argv[3]);
    }
    else if (strcmp(command, "recover") == 0)
    {
        recover_from_wal();
    }
    else if (strcmp(command, "display") == 0)
    {
        display_wal_and_db();
    }
    else
    {
        printf("Invalid command or missing arguments.\n");
    }

    return 0;
}
