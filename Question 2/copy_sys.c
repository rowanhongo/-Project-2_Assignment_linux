#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int in, out, size;
    long n;
    char *buffer;
    struct timespec start, end;
    double elapsed;

    if (argc != 4)
    {
        printf("Usage: %s <source> <destination> <buffer_size>\n", argv[0]);
        return 1;
    }

    size = atoi(argv[3]);

    if (size <= 0)
    {
        printf("Buffer size must be more than 0.\n");
        return 1;
    }

    in = open(argv[1], O_RDONLY);

    if (in < 0)
    {
        perror("open source");
        return 1;
    }

    out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (out < 0)
    {
        perror("open destination");
        close(in);
        return 1;
    }

    buffer = malloc(size);

    if (buffer == NULL)
    {
        perror("malloc");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    while ((n = read(in, buffer, size)) > 0)
    {
        if (write(out, buffer, n) != n)
        {
            perror("write");
            return 1;
        }
    }

    if (n < 0)
    {
        perror("read");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    free(buffer);
    close(in);
    close(out);

    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Elapsed time: %.6f seconds\n", elapsed);

    return 0;
}
