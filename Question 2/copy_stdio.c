#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    FILE *in, *out;
    int size;
    size_t n;
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

    in = fopen(argv[1], "rb");

    if (in == NULL)
    {
        perror("fopen source");
        return 1;
    }

    out = fopen(argv[2], "wb");

    if (out == NULL)
    {
        perror("fopen destination");
        fclose(in);
        return 1;
    }

    buffer = malloc(size);

    if (buffer == NULL)
    {
        perror("malloc");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    while ((n = fread(buffer, 1, size, in)) > 0)
    {
        if (fwrite(buffer, 1, n, out) != n)
        {
            perror("fwrite");
            return 1;
        }
    }

    if (ferror(in))
    {
        perror("fread");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    free(buffer);
    fclose(in);
    fclose(out);

    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Elapsed time: %.6f seconds\n", elapsed);

    return 0;
}
