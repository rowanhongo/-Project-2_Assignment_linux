#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

char *pattern;
char **files;
int fileCount;
int nextFile = 0;
long grandTotal = 0;
FILE *report;

pthread_mutex_t queueLock;
pthread_mutex_t reportLock;
pthread_mutex_t totalLock;

double now()
{
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec + t.tv_nsec / 1000000000.0;
}

void *worker(void *arg)
{
    int id = *(int *)arg;
    int index;
    long matching, total;
    char line[4096];
    FILE *fp;

    while (1)
    {
        // take the next file from the queue
        pthread_mutex_lock(&queueLock);
        index = nextFile;
        nextFile++;
        pthread_mutex_unlock(&queueLock);

        if (index >= fileCount)
            break;

        fp = fopen(files[index], "r");

        if (fp == NULL)
        {
            perror(files[index]);
            continue;
        }

        matching = 0;
        total = 0;

        while (fgets(line, sizeof(line), fp) != NULL)
        {
            total++;

            if (strstr(line, pattern) != NULL)
                matching++;
        }

        fclose(fp);

        pthread_mutex_lock(&reportLock);
        fprintf(report, "%s: %ld matching lines out of %ld (thread %d)\n", files[index], matching, total, id);
        pthread_mutex_unlock(&reportLock);

        pthread_mutex_lock(&totalLock);
        grandTotal += matching;
        pthread_mutex_unlock(&totalLock);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int i, numThreads;
    int ids[64];
    pthread_t threads[64];
    double start, elapsed;

    if (argc < 5)
    {
        printf("Usage: %s pattern report.txt <number_of_threads> file1.log file2.log ...\n", argv[0]);
        return 1;
    }

    pattern = argv[1];
    numThreads = atoi(argv[3]);
    files = &argv[4];
    fileCount = argc - 4;

    if (numThreads < 1 || numThreads > 64)
    {
        printf("Number of threads must be from 1 to 64.\n");
        return 1;
    }

    report = fopen(argv[2], "w");

    if (report == NULL)
    {
        perror(argv[2]);
        return 1;
    }

    pthread_mutex_init(&queueLock, NULL);
    pthread_mutex_init(&reportLock, NULL);
    pthread_mutex_init(&totalLock, NULL);

    start = now();

    for (i = 0; i < numThreads; i++)
    {
        ids[i] = i + 1;

        if (pthread_create(&threads[i], NULL, worker, &ids[i]) != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }

    for (i = 0; i < numThreads; i++)
        pthread_join(threads[i], NULL);

    elapsed = now() - start;

    fprintf(report, "Total matching lines across %d files: %ld\n", fileCount, grandTotal);
    printf("Total matching lines across %d files: %ld\n", fileCount, grandTotal);
    printf("Elapsed time: %.6f seconds\n", elapsed);

    fclose(report);

    pthread_mutex_destroy(&queueLock);
    pthread_mutex_destroy(&reportLock);
    pthread_mutex_destroy(&totalLock);

    return 0;
}
