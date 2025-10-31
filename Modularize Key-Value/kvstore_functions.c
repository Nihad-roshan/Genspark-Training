#include <string.h>
#include "kvstore.h"

#define MAX_ENTRIES 100

typedef struct
{
    char key[BUF_SIZE];
    char value[BUF_SIZE];
} keyvalue;

static keyvalue store[MAX_ENTRIES];
static int store_count = 0;

const char *kv_get(const char *key)
{
    for (int i = 0; i < store_count; i++)
    {
        if (strcmp(store[i].key, key) == 0)
            return store[i].value;
    }
    return NULL;
}

void kv_set(const char *key, const char *value)
{
    for (int i = 0; i < store_count; i++)
    {
        if (strcmp(store[i].key, key) == 0)
        {
            strncpy(store[i].value, value, BUF_SIZE - 1);
            store[i].value[BUF_SIZE - 1] = '\0';
            return;
        }
    }

    if (store_count < MAX_ENTRIES)
    {
        strncpy(store[store_count].key, key, BUF_SIZE - 1);
        store[store_count].key[BUF_SIZE - 1] = '\0';
        strncpy(store[store_count].value, value, BUF_SIZE - 1);
        store[store_count].value[BUF_SIZE - 1] = '\0';
        store_count++;
    }
}
