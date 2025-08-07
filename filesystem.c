#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#define CACHE_SIZE 5 //number of blocks
#define KEY_SIZE 32 //size of key
#define DATA_SIZE 256 //size of data
#define BUFFER_SIZE 4096 //size of each buffer
#define POOL_SIZE 10 //total number of buffers in buffer pool
//buffer structure individual
typedef struct buffer
{
    char data[BUFFER_SIZE];
    bool isfree;
    int bufferid;
    struct buffer *next;

}buffer_t;

//buffer pool manager
typedef struct buffer_pool
{
    buffer_t *buffers; //pointing to buffers
    buffer_t *freelist; //pointing to the first free buffer
    int totalbuffers; //total buffers created
    int freecount;  //available
}buffer_pool_t;

//lru ache structures
//stores file block and point to buffer holding its data
typedef struct cache_node
{
    char key[KEY_SIZE];
    buffer_t *buffers; //pointer tobuffer in memory pool
    struct cache_node *prev;
    struct cache_node *next;

}cache_node_t;

//lru cache manager
typedef struct lru_cache
{
    cache_node_t *head;  //mru
    cache_node_t *tail; //lru
    cache_node_t *nodes[CACHE_SIZE];  //for maching key
    int count;   //number of nodes currently in cache
    int capacity; //max numbers allowed

}lru_cache_t;

unsigned int hash(const char *key)
{
    unsigned int hashvalue =0;
    while(*key)
    {
        hashvalue=(hashvalue*31)+*key++;
    }
    return hashvalue%CACHE_SIZE;
}

//function of bufferpool

buffer_pool_t* createbufferpool(int poolsize)
{
    buffer_pool_t *pool=malloc(sizeof(buffer_pool_t));
    if(!pool) return NULL;
     
    pool->buffers=malloc(sizeof(buffer_t)*poolsize); //giving memory to individual struct contining data
    if(!pool->buffers)
    {
        free(pool);
        return NULL;
    }

    pool->totalbuffers=poolsize;
    pool->freecount=poolsize;
    pool->freelist=&pool->buffers[0];
   
    //initialized each buffer and link them into free list
    for(int i=0;i<poolsize;i++)
    {
        pool->buffers[i].isfree=true;
        pool->buffers[i].bufferid=i;
        pool->buffers[i].next=(i==poolsize-1)?NULL:&pool->buffers[i+1];

    }

return pool;

}

buffer_t* acquire_buffer(buffer_pool_t *pool)
{
    if(pool->freecount==0)
    {
        printf("buffer pool exhausted!\n");
        return NULL;

    }

    //accessing the first freebuffer from the list
    buffer_t *buf=pool->freelist;
    pool->freelist=buf->next;
    buf->isfree=false;
    buf->next=NULL;
    pool->freecount--;

    printf("Acquired buffer %d\n",buf->bufferid);
    return buf;

}

void release_buffer(buffer_pool_t *pool, buffer_t *buffer)
{
    if(!buffer || buffer->isfree) return;

    memset(buffer->data,0,BUFFER_SIZE);
    buffer->isfree=true;

    //making buffer to the front of the free list
    buffer->next=pool->freelist;
    pool->freelist=buffer;
    pool->freecount++;

    printf("released buffer %d\n",buffer->bufferid);

}

//distroying buffer pool and free memory

void destroybufferpool(buffer_pool_t *pool)
{
    if(pool)
    {
        free(pool->buffers);
        free(pool);
    }
}

//lru cache functions

//creating initial lrucache

lru_cache_t *create_lru_cache()
{
    lru_cache_t *cache=malloc(sizeof(lru_cache_t));
    if(!cache) return NULL;


    cache->head=NULL;
    cache->tail=NULL;
    cache->count=0;
    cache->capacity=CACHE_SIZE;

    for(int i=0;i<CACHE_SIZE;i++)
    {
        cache->nodes[i]=NULL;
    }
    return cache;
}

//moving the node into most recently used as front

void movetofront(lru_cache_t *cache,cache_node_t *node)
{
    if(cache->head==node) return;
    //cleared node
    if(node->prev) node->prev->next =node->next;
    if(node->next) node->next->prev=node->prev;
    if(cache->tail==node) cache->tail=node->prev;

    //moving to front
    node->prev=NULL;
    node->next=cache->head;
    if(cache->head) cache->head->prev=node;
    cache->head=node;

    if(!cache->tail) cache->tail=node;

}

//adding a node to front of a cache

void addtofront(lru_cache_t *cache,cache_node_t *node)
{
    node->prev=NULL;
    node->next=cache->head;

    if(cache->head) cache->head->prev=node;
    cache->head=node;
    if(!cache->tail) cache->tail=node;
}

//remove least recently like tail node from cache

cache_node_t *removelru(lru_cache_t *cache)
{
    if(!cache->tail) return NULL;

    cache_node_t *lru=cache->tail;

    //detaching node


    if(lru->prev)
    lru->prev->next=NULL;
    else
    cache->head=NULL;

    cache->tail=lru->prev;

    return lru;
}
// get buffer from cache if key is present

buffer_t *cache_get(lru_cache_t *cache,const char *key)
{
    unsigned int index=hash(key);
    cache_node_t *current=cache->nodes[index];

    while(current)
    {
        if(strcmp(current->key,key)==0)
        {
            movetofront(cache,current);
            printf("cache HIT:%s\n",key);
            return current->buffers;
        }
        current=current->next;
    }
    printf("cache Miss: %s\n",key);
    return NULL;

}

void cacheput(lru_cache_t *cache,buffer_pool_t *pool,const char *key,buffer_t *buffer)
{
    unsigned int index=hash(key);

    //checking if already present
    cache_node_t *current =cache->nodes[index];
    while(current)
    {
        if(strcmp(current->key,key)==0)
        {
            movetofront(cache,current);
            return;

        }
        current=current->next;
    }

    //evicting incase of full evict lru
    if(cache->count >= cache->capacity)
    {
        cache_node_t *lru = removelru(cache);
        if(lru)
        {
            unsigned int lru_index=hash(lru->key);
            if(cache->nodes[lru_index]==lru)
            cache->nodes[lru_index]=lru->next;

            if(lru->buffers)
            release_buffer(pool,lru->buffers);

            free(lru);
            cache->count--;

        }
    }
    //allocating new node
    cache_node_t *new_node=malloc(sizeof(cache_node_t));
    if(!new_node) return;

    strncpy(new_node->key,key,KEY_SIZE-1);
    new_node->key[KEY_SIZE-1]='\0';
    new_node->buffers=buffer;
    new_node->prev=new_node->next=NULL;

    //adding caceh and hash bucket

    new_node->next=cache->nodes[index];
    cache->nodes[index]=new_node;
    addtofront(cache,new_node);
    cache->count++;

    printf("cached block:%s in buffer %d\n",key,buffer->bufferid);

}

//files

void files(buffer_pool_t *pool,lru_cache_t *cache,const char *block_key,const char *data)
{
//trying to get from cache first
buffer_t *cached_buffer=cache_get(cache,block_key);
if(cached_buffer)
{
    printf("read from cache %s:%s\n",block_key,cached_buffer->data);
    return;

}
//when cache miss we go for buffer
    buffer_t *buffer=acquire_buffer(pool);
    if(!buffer) return;

    strncpy(buffer->data,data,BUFFER_SIZE-1);
    buffer->data[BUFFER_SIZE-1]='\0';

//caching buffer
    cacheput(cache,pool,block_key,buffer);
    printf("loaded and cached block %s:%s\n",block_key,data);


}
int main()
{
      
    buffer_pool_t *pool = createbufferpool(POOL_SIZE); //created bufferpool
    lru_cache_t *cache = create_lru_cache(); //cache system created

  
    files(pool, cache, "file1_block1", "a1");
    files(pool, cache, "file2_block1", "a2");
    files(pool, cache, "file3_block1", "a3");
    files(pool, cache, "file1_block1", " hit cache");

    
    files(pool, cache, "file4_block1", "a4");
    files(pool, cache, "file5_block1", "a5");
    files(pool, cache, "file6_block1", "Eviction..");

    // Clean up memory
 //   

    // Free LRU cache memory
cache_node_t *current = cache->head;
while (current) {
    cache_node_t *next = current->next;
    if (current->buffers && !current->buffers->isfree)
        release_buffer(pool, current->buffers);
    free(current);
    current = next;
}
free(cache);

    return 0;
}
