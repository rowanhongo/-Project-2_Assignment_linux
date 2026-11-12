#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define THREADS 8
#define MAX 100000

struct Range
{
    int id;
    int start;
    int end;
    double time;
};

pthread_mutex_t lock;
int globalCount = 0;
int found[100];
int foundCount = 0;

double now()
{
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec + t.tv_nsec / 1000000000.0;
}

// checks divisors only up to sqrt(n)
int isPerfect(int n)
{
    int i;
    int sum = 1;

    if (n < 2)
        return 0;

    for (i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
        {
            sum += i;

            if (i != n / i)
                sum += n / i;
        }
    }

    return sum == n;
}

void *worker(void *arg)
{
    struct Range *r = (struct Range *)arg;
    int n;
    int localCount = 0;
    double begin = now();

    for (n = r->start; n <= r->end; n++)
    {
        if (isPerfect(n))
        {
            localCount++;

            pthread_mutex_lock(&lock);
            found[foundCount] = n;
            foundCount++;
            pthread_mutex_unlock(&lock);
        }
    }

    // add to the shared counter once at the end
    pthread_mutex_lock(&lock);
    globalCount += localCount;
    pthread_mutex_unlock(&lock);

    r->time = now() - begin;

    return NULL;
}

int main()
{
    pthread_t threads[THREADS];
    struct Range ranges[THREADS];
    int i, j, temp, n;
    int singleCount = 0;
    double singleStart, singleTime, multiStart, multiTime;
    int size = MAX / THREADS;

    pthread_mutex_init(&lock, NULL);

    // single threaded version
    singleStart = now();

    for (n = 1; n <= MAX; n++)
    {
        if (isPerfect(n))
            singleCount++;
    }

    singleTime = now() - singleStart;

    // multi threaded version
    multiStart = now();

    for (i = 0; i < THREADS; i++)
    {
        ranges[i].id = i + 1;
        ranges[i].start = i * size + 1;
        ranges[i].end = (i + 1) * size;

        if (pthread_create(&threads[i], NULL, worker, &ranges[i]) != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }

    for (i = 0; i < THREADS; i++)
        pthread_join(threads[i], NULL);

    multiTime = now() - multiStart;

    pthread_mutex_destroy(&lock);

    // threads finish in any order so sort the array
    for (i = 0; i < foundCount - 1; i++)
    {
        for (j = 0; j < foundCount - i - 1; j++)
        {
            if (found[j] > found[j + 1])
            {
                temp = found[j];
                found[j] = found[j + 1];
                found[j + 1] = temp;
            }
        }
    }

    printf("Perfect numbers found:");

    for (i = 0; i < foundCount; i++)
        printf(" %d", found[i]);

    printf("\n");
    printf("The synchronized total number of perfect numbers between 1 and %d is %d\n", MAX, globalCount);
    printf("Single-threaded time: %.6f seconds (found %d)\n", singleTime, singleCount);
    printf("Multi-threaded time:  %.6f seconds\n", multiTime);

    printf("\nTime for each thread:\n");

    for (i = 0; i < THREADS; i++)
        printf("Thread %d (%d to %d): %.6f seconds\n", ranges[i].id, ranges[i].start, ranges[i].end, ranges[i].time);

    return 0;
}
