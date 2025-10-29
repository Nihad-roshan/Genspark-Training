#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define maxrows 6

typedef struct
{
    int id;
    char studentname[256];
    char subject[256];
    int score;
} record;

record records[maxrows] = {
    {101, "abc", "math", 90},
    {102, "def", "math", 70},
    {103, "ghi", "math", 65},
    {104, "jkl", "math", 84},
    {105, "mno", "math", 20},
    {106, "xyz", "math", 100}};

int rowindex;

// table scan
void scanopen();
record *scannext();
void scanclose();

// filter
void filtopen();
record *filtnext();
void filtclose();

// projection
void projopen();
void projnext();
void projclose();

void scanopen()
{
    rowindex = 0;
    printf("scan opened...\n");
}

record *scannext()
{
    if (rowindex >= maxrows)
        return NULL;
    return &records[rowindex++];
}

void scanclose()
{
    rowindex = 0;
    printf("scan closed.\n");
}

// filter
void filtopen()
{
    scanopen();
    printf("filter opened...\n");
}

record *filtnext()
{
    record *rec = NULL;
    while ((rec = scannext()) != NULL)
    {
        if ((rec->score > 70) && (strcmp(rec->subject, "math") == 0))
        {
            return rec;
        }
    }
    return rec;
}

void filtclose()
{
    scanclose();
    printf("filter closed.\n");
}

// projection
void projopen()
{
    filtopen();
    printf("projection opened...\n");
}

void projnext()
{
    record *rec = NULL;
    while ((rec = filtnext()) != NULL)
    {
        printf("student: %s   score: %d\n", rec->studentname, rec->score);
    }
}

void projclose()
{
    filtclose();
    printf("projection closed.\n");
}

int main()
{
    projopen();
    projnext();
    projclose();
    return 0;
}
