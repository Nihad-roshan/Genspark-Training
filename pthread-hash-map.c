#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define TABLE_SIZE 8  // number of buckets
#define NUM_THREADS 4 // example thread count

// Node in linked list for each bucket
typedef struct Node
{
    char *key;
    int value;
    struct Node *next;
} Node;

// Hash map structure
typedef struct
{
    Node *buckets[TABLE_SIZE];
    pthread_mutex_t locks[TABLE_SIZE]; // one lock per bucket
} hashmap;

// Hash function (djb2 algorithm)
unsigned long hash(const char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
}

// Initialize hash map
void hashmap_init(hashmap *map)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        map->buckets[i] = NULL;
        pthread_mutex_init(&map->locks[i], NULL);
    }
}

// Insert key–value into hash map (thread-safe)
void hashmap_insert(hashmap *map, const char *key, int value)
{
    unsigned long index = hash(key);
    pthread_mutex_lock(&map->locks[index]); // lock bucket

    Node *node = map->buckets[index];
    while (node)
    {
        if (strcmp(node->key, key) == 0)
        {
            node->value = value; // update existing
            pthread_mutex_unlock(&map->locks[index]);
            return;
        }
        node = node->next;
    }

    // Insert new node at head
    Node *new_node = malloc(sizeof(Node));
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = map->buckets[index];
    map->buckets[index] = new_node;

    pthread_mutex_unlock(&map->locks[index]); // unlock bucket
}

// Lookup key in hash map (thread-safe)
int hashmap_lookup(hashmap *map, const char *key, int *found)
{
    unsigned long index = hash(key);
    pthread_mutex_lock(&map->locks[index]); // lock bucket

    Node *node = map->buckets[index];
    while (node)
    {
        if (strcmp(node->key, key) == 0)
        {
            int val = node->value;
            pthread_mutex_unlock(&map->locks[index]);
            *found = 1;
            return val;
        }
        node = node->next;
    }

    pthread_mutex_unlock(&map->locks[index]); // unlock bucket
    *found = 0;
    return -1;
}

// Example thread function
void *worker(void *arg)
{
    hashmap *map = (hashmap *)arg;
    char key[32];

    // Insert some keys
    for (int i = 0; i < 5; i++)
    {
        sprintf(key, "key_%d", i);
        hashmap_insert(map, key, i * 100);
        printf("Thread %ld inserted: %s -> %d\n", pthread_self(), key, i * 100);
    }

    // Lookup keys
    for (int i = 0; i < 5; i++)
    {
        sprintf(key, "key_%d", i);
        int found;
        int val = hashmap_lookup(map, key, &found);
        if (found)
        {
            printf("Thread %ld looked up: %s -> %d\n", pthread_self(), key, val);
        }
    }

    return NULL;
}

int main()
{
    hashmap map;
    hashmap_init(&map);

    pthread_t threads[NUM_THREADS];

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, worker, &map);
    }

    // Wait for threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
