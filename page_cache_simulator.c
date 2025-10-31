#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAMES 3
#define MAX_PAGES 12

int pages[MAX_PAGES] = {1, 2, 3, 1, 2, 3, 1, 4, 2, 5, 1, 2};

// LRU IMPLEMENTATION 

int findPage(int buffer[], int page, int n)
{
    for (int i = 0; i < n; i++)
        if (buffer[i] == page)
            return i;
    return -1;
}

void printBuffer(int buffer[], int n)
{
    printf("[");
    for (int i = 0; i < n; i++)
    {
        if (buffer[i] != -1)
            printf("%d", buffer[i]);
        else
            printf("-");
        if (i < n - 1)
            printf(" ");
    }
    printf("]");
}

void simulateLRU()
{
    int buffer[MAX_FRAMES];
    int time[MAX_FRAMES];
    int hits = 0, misses = 0, counter = 0;

    for (int i = 0; i < MAX_FRAMES; i++)
    {
        buffer[i] = -1;
        time[i] = -1;
    }

    printf("\n--> LRU Page Replacement Simulation \n");

    for (int i = 0; i < MAX_PAGES; i++)
    {
        int page = pages[i];
        int pos = findPage(buffer, page, MAX_FRAMES);

        printf("Request %2d -> Page %d: ", i + 1, page);

        if (pos != -1)
        {
            hits++;
            time[pos] = ++counter;
            printf("HIT ");
        }
        else
        {
            misses++;
            int empty = -1;

            for (int j = 0; j < MAX_FRAMES; j++)
            {
                if (buffer[j] == -1)
                {
                    empty = j;
                    break;
                }
            }

            if (empty == -1)
            {
                int lru = 0;
                for (int j = 1; j < MAX_FRAMES; j++)
                    if (time[j] < time[lru])
                        lru = j;

                printf("MISS (Evict %d) ", buffer[lru]);
                buffer[lru] = page;
                time[lru] = ++counter;
            }
            else
            {
                printf("MISS (Insert) ");
                buffer[empty] = page;
                time[empty] = ++counter;
            }
        }

        printf("Buffer: ");
        printBuffer(buffer, MAX_FRAMES);
        printf("\n");
    }

    printf("\nTotal Hits: %d\nTotal Misses: %d\nHit Ratio: %.2f\n", hits, misses, (float)hits / (hits + misses));
}


// CLOCK IMPLEMENTATION
void simulateClock()
{
    int buffer[MAX_FRAMES];
    int refBit[MAX_FRAMES];
    int pointer = 0, hits = 0, misses = 0;

    for (int i = 0; i < MAX_FRAMES; i++)
    {
        buffer[i] = -1;
        refBit[i] = 0;
    }

    printf("\n--> Clock Page Replacement Simulation\n");

    for (int i = 0; i < MAX_PAGES; i++)
    {
        int page = pages[i];
        int found = 0;

        for (int j = 0; j < MAX_FRAMES; j++)
        {
            if (buffer[j] == page)
            {
                found = 1;
                refBit[j] = 1;
                hits++;
                break;
            }
        }

        if (!found)
        {
            misses++;
            while (1)
            {
                if (refBit[pointer] == 0)
                {
                    buffer[pointer] = page;
                    refBit[pointer] = 1;
                    pointer = (pointer + 1) % MAX_FRAMES;
                    break;
                }
                else
                {
                    refBit[pointer] = 0;
                    pointer = (pointer + 1) % MAX_FRAMES;
                }
            }
        }

        printf("Request %2d -> ", page);
        for (int j = 0; j < MAX_FRAMES; j++)
        {
            if (buffer[j] != -1)
                printf("%d(%d) ", buffer[j], refBit[j]);
            else
                printf("- ");
        }
        printf("\n");
    }

    printf("\nTotal Hits: %d\nTotal Misses: %d\nHit Ratio: %.2f\n", hits, misses, (float)hits / (hits + misses));
}




// ARC IMPLEMENTATION 

typedef struct Node
{
    int page;
    struct Node *next;
} Node;

Node *T1 = NULL, *T2 = NULL, *B1 = NULL, *B2 = NULL;
int p = 0, hitsARC = 0, missesARC = 0;

int contains(Node *list, int page)
{
    for (Node *cur = list; cur; cur = cur->next)
        if (cur->page == page)
            return 1;
    return 0;
}

void removeNode(Node **list, int page) 
{
    Node *cur = *list, *prev = NULL;
    while (cur)
    {
        if (cur->page == page)
        {
            if (prev)
                prev->next = cur->next;
            else
                *list = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void addToFront(Node **list, int page)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->page = page;
    newNode->next = *list;
    *list = newNode;
}

int length(Node *list)
{
    int len = 0;
    for (Node *cur = list; cur; cur = cur->next)
        len++;
    return len;
}

int removeLRU(Node **list)
{
    if (*list == NULL)
        return -1;
    Node *cur = *list, *prev = NULL;
    if (cur->next == NULL)
    {
        int page = cur->page;
        free(cur);
        *list = NULL;
        return page;
    }
    while (cur->next)
    {
        prev = cur;
        cur = cur->next;
    }
    int page = cur->page;
    free(cur);
    prev->next = NULL;
    return page;
}

void replaceARC()
{
    if (length(T1) >= 1 && ((length(T1) > p) || (contains(B2, 0) && length(T1) == p)))
    {
        int old = removeLRU(&T1);
        addToFront(&B1, old);
    }
    else
    {
        int old = removeLRU(&T2);
        addToFront(&B2, old);
    }
}

void simulateARC()
{
    printf("\n-->ARC Page Replacement Simulation \n");

    for (int i = 0; i < MAX_PAGES; i++)
    {
        int page = pages[i];
        printf("Request %2d -> Page %d: ", i + 1, page);

        if (contains(T1, page) || contains(T2, page))
        {
            hitsARC++;
            if (contains(T1, page))
            {
                removeNode(&T1, page);
                addToFront(&T2, page);
            }
            else
            {
                removeNode(&T2, page);
                addToFront(&T2, page);
            }
            printf("HIT\n");
            continue;
        }

        missesARC++;

        if (contains(B1, page))
        {
            p = (p + (length(B2) > 0 ? 1 : 0)) < MAX_FRAMES ? p + 1 : MAX_FRAMES;
            replaceARC();
            removeNode(&B1, page);
            addToFront(&T2, page);
            printf("MISS (from B1 -> T2)\n");
        }
        else if (contains(B2, page))
        {
            p = (p - (length(B1) > 0 ? 1 : 0)) > 0 ? p - 1 : 0;
            replaceARC();
            removeNode(&B2, page);
            addToFront(&T2, page);
            printf("MISS (from B2 -> T2)\n");
        }
        else
        {
            if (length(T1) + length(T2) >= MAX_FRAMES)
                replaceARC();
            addToFront(&T1, page);
            printf("MISS (new -> T1)\n");
        }

        printf("T1: ");
        for (Node *cur = T1; cur; cur = cur->next)
            printf("%d ", cur->page);
        printf("| T2: ");
        for (Node *cur = T2; cur; cur = cur->next)
            printf("%d ", cur->page);
        printf("| B1: ");
        for (Node *cur = B1; cur; cur = cur->next)
            printf("%d ", cur->page);
        printf("| B2: ");
        for (Node *cur = B2; cur; cur = cur->next)
            printf("%d ", cur->page);
        printf("\n");
    }

    printf("\nTotal Hits: %d\nTotal Misses: %d\nHit Ratio: %.2f\n", hitsARC, missesARC, (float)hitsARC / (hitsARC + missesARC));
}


// MAIN SELECTION

int main()
{
    int choice;
    printf("Select Replacement Policy:\n");
    printf("1. LRU\n2. Clock\n3. ARC\nEnter choice: ");
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
        simulateLRU();
        break;
    case 2:
        simulateClock();
        break;
    case 3:
        simulateARC();
        break;
    default:
        printf("Invalid choice!\n");
    }
    return 0;
}
