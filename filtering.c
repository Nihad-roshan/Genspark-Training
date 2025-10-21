#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define num_records 1000

typedef struct
{
    int id;
    char name[20];
    int age;
} record;


void generaterecords(record records[], int n)
{//for generating random records
    for (int i = 0; i < n; i++)
    {
        records[i].id = i + 1; // id 
        sprintf(records[i].name, "name%d", i + 1);
        records[i].age = rand() % 100; // random age 0-99
    }
}

void linearsearch(record records[], int n, int minage)
{//for age >= condition
    for (int i = 0; i < n; i++)
    {
        if (records[i].age >= minage)
        {
            printf("id: %d, name: %s, age: %d\n", records[i].id, records[i].name, records[i].age);
        }
    }
}

int comparebyage(const void *a, const void *b)
{
    record *ra = (record *)a;
    record *rb = (record *)b;
    return ra->age - rb->age;
}


int binarysearch(record records[], int n, int minage)
{//for first record with age >= minage
    int left = 0, right = n - 1;
    int result = -1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        if (records[mid].age >= minage)
        {
            result = mid;
            right = mid - 1; // search left for first occurrence
        }
        else
        {
            left = mid + 1;
        }
    }
    return result;
}

int main()
{
    record records[num_records];
    srand(time(NULL));

    int filterage = 50; // filter condition


    generaterecords(records, num_records);

    // for ls
    clock_t start = clock();
    linearsearch(records, num_records, filterage);
    clock_t end = clock();
    double linear_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    // for bs
    qsort(records, num_records, sizeof(record), comparebyage);

    start = clock();
    int index = binarysearch(records, num_records, filterage);
    end = clock();
    double binary_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("linear search time: %lf seconds\n", linear_time);
    printf("binary search time: %lf seconds\n", binary_time);

    if (index != -1)
    {
        printf("first record with age >= %d is id: %d, age: %d\n",filterage, records[index].id, records[index].age);
    }
    else
    {
        printf("no record found with age >= %d\n", filterage);
    }

    return 0;
}
