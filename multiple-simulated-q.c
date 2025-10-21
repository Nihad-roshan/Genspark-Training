#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_QUERIES 5

typedef struct
{
    char query[100];
    int cost; // lower cost means more efficient
} Query;

// Function to simulate query execution
void execute_query(Query q)
{
    printf("Executing query: %s (cost: %d)\n", q.query, q.cost);
    // Simulate execution time proportional to cost
    usleep(q.cost * 100000); // microseconds
}

// Comparison function for sorting by cost
int compare_queries(const void *a, const void *b)
{
    Query *qa = (Query *)a;
    Query *qb = (Query *)b;
    return qa->cost - qb->cost;
}

int main()
{
    Query queries[NUM_QUERIES] = {
        {"SELECT * FROM table WHERE age > 50", 5},
        {"SELECT * FROM table WHERE name LIKE 'A%'", 2},
        {"SELECT * FROM table WHERE salary > 50000", 4},
        {"SELECT * FROM table WHERE id < 100", 1},
        {"SELECT * FROM table WHERE age BETWEEN 20 AND 30", 3}};

    clock_t start, end;
    double execution_time;

    printf("Simulated queries before optimization:\n");
    for (int i = 0; i < NUM_QUERIES; i++)
    {
        printf("%d: %s (cost: %d)\n", i + 1, queries[i].query, queries[i].cost);
    }

    // Sort queries by cost (most efficient first)
    qsort(queries, NUM_QUERIES, sizeof(Query), compare_queries);

    printf("\nExecuting queries in optimized order:\n");

    start = clock();
    for (int i = 0; i < NUM_QUERIES; i++)
    {
        execute_query(queries[i]);
    }
    end = clock();

    execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nTotal execution time: %lf seconds\n", execution_time);

    return 0;
}
