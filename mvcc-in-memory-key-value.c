
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>
#include <stdatomic.h>
#include <unistd.h>

#define MAX_KEYS 5
#define MAX_TRANSACTIONS 5
#define MAX_WRITES_PER_TX 8
#define KEY_NAME_LEN 16

typedef struct Version
{//MVCC
    char value[32];
    unsigned long begin_ts;
    unsigned long end_ts;  
    struct Version *next; 
} Version;


typedef struct
{//key-value
    char name[KEY_NAME_LEN];
    Version *versions; // newest-first
    int locked_by;     // -1 if free, otherwise tx id (simple single-writer lock)
} KVPair;

typedef enum
{
    TX_UNUSED = 0,
    TX_ACTIVE = 1,
    TX_ABORTED = 2,
    TX_COMMITTED = 3
} TxState;
typedef struct
{
    int id;
    TxState state;
    unsigned long start_ts; 
    int write_count;
    int write_keys[MAX_WRITES_PER_TX];
    char write_values[MAX_WRITES_PER_TX][32];
} Transaction;

KVPair store[MAX_KEYS];
Transaction txs[MAX_TRANSACTIONS];
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
int wait_for[MAX_TRANSACTIONS][MAX_TRANSACTIONS];        
atomic_ulong global_ts = 1;

int find_key_index(const char *name)
{
    for (int i = 0; i < MAX_KEYS; ++i)
        if (strcmp(store[i].name, name) == 0)
            return i;
    return -1;
}

void init_store(void)
{
    pthread_mutex_lock(&global_lock);
    for (int i = 0; i < MAX_KEYS; ++i)
    {
        snprintf(store[i].name, KEY_NAME_LEN, "key%d", i);
        // initial version with value "-"
        Version *v = malloc(sizeof(Version));
        strcpy(v->value, "-");
        v->begin_ts = 1;
        v->end_ts = ULONG_MAX;
        v->next = NULL;
        store[i].versions = v;
        store[i].locked_by = -1;
    }
    for (int i = 0; i < MAX_TRANSACTIONS; ++i)
    {
        txs[i].id = i;
        txs[i].state = TX_UNUSED;
        txs[i].write_count = 0;
    }
    // clear wait-for
    for (int i = 0; i < MAX_TRANSACTIONS; ++i)
        for (int j = 0; j < MAX_TRANSACTIONS; ++j)
            wait_for[i][j] = 0;
    pthread_mutex_unlock(&global_lock);
}


bool dfs_cycle(int u, int visited[], int recStack[])
{//deadlock-detection
    visited[u] = 1;
    recStack[u] = 1;
    for (int v = 0; v < MAX_TRANSACTIONS; ++v)
    {
        if (wait_for[u][v])
        {
            if (!visited[v] && dfs_cycle(v, visited, recStack))
                return true;
            else if (recStack[v])
                return true;
        }
    }
    recStack[u] = 0;
    return false;
}

bool detect_deadlock()
{
    int visited[MAX_TRANSACTIONS] = {0}, recStack[MAX_TRANSACTIONS] = {0};
    for (int i = 0; i < MAX_TRANSACTIONS; ++i)
        if (!visited[i] && dfs_cycle(i, visited, recStack))
            return true;
    return false;
}

Transaction *begin_tx(int id)
{
    pthread_mutex_lock(&global_lock);
    if (id < 0 || id >= MAX_TRANSACTIONS)
    {
        pthread_mutex_unlock(&global_lock);
        return NULL;
    }
    Transaction *t = &txs[id];
    t->id = id;
    t->state = TX_ACTIVE;
    t->start_ts = atomic_fetch_add(&global_ts, 1);
    t->write_count = 0;
    // clear any previous wait_for edges for safety
    for (int i = 0; i < MAX_TRANSACTIONS; ++i)
        wait_for[id][i] = wait_for[i][id] = 0;
    pthread_mutex_unlock(&global_lock);
    printf("[TX%d] begun (snapshot ts=%lu)\n", id, t->start_ts);
    return t;
}

// Try to acquire write lock for key. Returns 0 -> acquired | 1 -> blocked (added wait-for edge, no cycle detected)| -1 -> deadlock detected (caller should abort)
int try_acquire_lock(Transaction *t, int kidx)
{
    pthread_mutex_lock(&global_lock);
    int owner = store[kidx].locked_by;
    if (owner == -1 || owner == t->id)
    {
        store[kidx].locked_by = t->id;
        // clear wait edges from this tx (it got what it wanted)
        for (int j = 0; j < MAX_TRANSACTIONS; ++j)
            wait_for[t->id][j] = 0;
        pthread_mutex_unlock(&global_lock);
        return 0;
    }
    else
    {
        // blocked: set wait-for edge t->owner and detect cycle
        wait_for[t->id][owner] = 1;
        bool cycle = detect_deadlock();
        pthread_mutex_unlock(&global_lock);
        if (cycle)
            return -1;
        return 1;
    }
}

int tx_read(Transaction *t, const char *kname, char *out_value, size_t out_len)
{
    if (!t || t->state != TX_ACTIVE)
        return -1;
    int kidx = find_key_index(kname);
    if (kidx < 0)
        return -1;

    pthread_mutex_lock(&global_lock);

    for (int i = 0; i < t->write_count; ++i)
    {
        if (t->write_keys[i] == kidx)
        {
            strncpy(out_value, t->write_values[i], out_len);
            pthread_mutex_unlock(&global_lock);
            return 0;
        }
    }

    Version *v = store[kidx].versions;
    while (v)
    {
        if (v->begin_ts <= t->start_ts && t->start_ts < v->end_ts)
        {
            strncpy(out_value, v->value, out_len);
            pthread_mutex_unlock(&global_lock);
            return 0;
        }
        v = v->next;
    }
    pthread_mutex_unlock(&global_lock);
    return -1;
}

int tx_write(Transaction *t, const char *kname, const char *value)
{
    if (!t || t->state != TX_ACTIVE)
        return -1;
    int kidx = find_key_index(kname);
    if (kidx < 0)
        return -1;

    int lock_res = try_acquire_lock(t, kidx);
    if (lock_res == -1)
    {

        printf("[TX%d] deadlock detected while locking %s -> aborting\n", t->id, kname);
        t->state = TX_ABORTED;
        pthread_mutex_lock(&global_lock);
        for (int j = 0; j < MAX_TRANSACTIONS; ++j)
            wait_for[t->id][j] = wait_for[j][t->id] = 0;
        for (int i = 0; i < MAX_KEYS; ++i)
            if (store[i].locked_by == t->id)
                store[i].locked_by = -1;
        pthread_mutex_unlock(&global_lock);
        return -1;
    }
    else if (lock_res == 1)
    {
        printf("[TX%d] blocked acquiring lock on %s (owner=%d). Write not buffered.\n", t->id, kname, store[kidx].locked_by);
        return 1;
    }

    pthread_mutex_lock(&global_lock);

    for (int i = 0; i < t->write_count; ++i)
    {
        if (t->write_keys[i] == kidx)
        {
            strncpy(t->write_values[i], value, sizeof(t->write_values[i]));
            pthread_mutex_unlock(&global_lock);
            printf("[TX%d] buffered update %s = %s (updated)\n", t->id, kname, value);
            return 0;
        }
    }

    if (t->write_count >= MAX_WRITES_PER_TX)
    {
        pthread_mutex_unlock(&global_lock);
        printf("[TX%d] write buffer full\n", t->id);
        return -1;
    }
    t->write_keys[t->write_count] = kidx;
    strncpy(t->write_values[t->write_count], value, sizeof(t->write_values[t->write_count]));
    t->write_count++;
    pthread_mutex_unlock(&global_lock);
    printf("[TX%d] buffered update %s = %s\n", t->id, kname, value);
    return 0;
}

int tx_commit(Transaction *t)
{
    if (!t || t->state != TX_ACTIVE)
        return -1;

    unsigned long commit_ts = atomic_fetch_add(&global_ts, 1);

    pthread_mutex_lock(&global_lock);

    for (int i = 0; i < t->write_count; ++i)
    {
        int kidx = t->write_keys[i];
        Version *newv = malloc(sizeof(Version));
        strncpy(newv->value, t->write_values[i], sizeof(newv->value));
        newv->begin_ts = commit_ts;
        newv->end_ts = ULONG_MAX;
        // retire old head by setting its end_ts to commit_ts
        Version *old = store[kidx].versions;
        if (old)
            old->end_ts = commit_ts;
        newv->next = old;
        store[kidx].versions = newv;
    }

    for (int i = 0; i < MAX_KEYS; ++i)
        if (store[i].locked_by == t->id)
            store[i].locked_by = -1;

    for (int j = 0; j < MAX_TRANSACTIONS; ++j)
        wait_for[t->id][j] = wait_for[j][t->id] = 0;

    t->state = TX_COMMITTED;
    pthread_mutex_unlock(&global_lock);

    printf("[TX%d] committed (commit_ts=%lu)\n", t->id, commit_ts);
    return 0;
}

// Abort: release locks, clear writes, clear wait-for edges
void tx_abort(Transaction *t)
{
    if (!t)
        return;
    pthread_mutex_lock(&global_lock);
    for (int i = 0; i < MAX_KEYS; ++i)
        if (store[i].locked_by == t->id)
            store[i].locked_by = -1;
    for (int j = 0; j < MAX_TRANSACTIONS; ++j)
    {
        wait_for[t->id][j] = 0;
        wait_for[j][t->id] = 0;
    }
    t->write_count = 0;
    t->state = TX_ABORTED;
    pthread_mutex_unlock(&global_lock);
    printf("[TX%d] aborted\n", t->id);
}

void dump_store(void)
{
    pthread_mutex_lock(&global_lock);
    printf("=== STORE DUMP ===\n");
    for (int i = 0; i < MAX_KEYS; ++i)
    {
        printf("%s (locked_by=%d): ", store[i].name, store[i].locked_by);
        Version *v = store[i].versions;
        while (v)
        {
            printf("[val=%s b=%lu e=%s] -> ", v->value, v->begin_ts, (v->end_ts == ULONG_MAX) ? "INF" : "num");
            v = v->next;
        }
        printf("\n");
    }
    printf("==================\n");
    pthread_mutex_unlock(&global_lock);
}


int main(void)
{
    init_store();
    dump_store();

    Transaction *t0 = begin_tx(0);
    Transaction *t1 = begin_tx(1);


    char buf[32];
    tx_read(t0, "key0", buf, sizeof(buf));
    printf("[TX0] read key0 => %s (snapshot ts=%lu)\n", buf, t0->start_ts);

    tx_write(t1, "key0", "value_from_t1");
    tx_commit(t1); // create new version at commit_ts

    tx_read(t0, "key0", buf, sizeof(buf));
    printf("[TX0] read key0 again => %s (snapshot ts=%lu) -- should be unchanged by T1 commit\n", buf, t0->start_ts);

    tx_write(t0, "key1", "t0_writes_key1");
    tx_write(t1, "key2", "t1_writes_key2");


    int r0 = tx_write(t0, "key2", "t0_try_key2");
    int r1 = tx_write(t1, "key1", "t1_try_key1");

    // Check results
    if (t0->state == TX_ACTIVE)
        tx_commit(t0);
    else
        tx_abort(t0);
    if (t1->state == TX_ACTIVE)
        tx_commit(t1);
    else
        tx_abort(t1);

    dump_store();
    return 0;
}

