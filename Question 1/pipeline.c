#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    // cat /etc/passwd | cut -d: -f7 | sort -u
    char *stage1[] = {"cat", "/etc/passwd", NULL};
    char *stage2[] = {"cut", "-d:", "-f7", NULL};
    char *stage3[] = {"sort", "-u", NULL};
    char **stages[3] = {stage1, stage2, stage3};

    int pipes[3][2];
    int pids[3];
    int status, fd, n, i, j;
    int lines = 0;
    int shown = 0;
    char buffer[4096];

    for (i = 0; i < 3; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            perror("pipe");
            return 1;
        }
    }

    for (i = 0; i < 3; i++)
    {
        pids[i] = fork();

        if (pids[i] < 0)
        {
            perror("fork");
            return 1;
        }

        if (pids[i] == 0)
        {
            // first stage reads from the terminal, the others read from the pipe before them
            if (i > 0)
                dup2(pipes[i - 1][0], STDIN_FILENO);

            dup2(pipes[i][1], STDOUT_FILENO);

            // close every pipe end so EOF can reach the next stage
            for (j = 0; j < 3; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(stages[i][0], stages[i]);
            perror("execvp");
            exit(1);
        }
    }

    // parent only needs the read end of the last pipe
    for (i = 0; i < 2; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    close(pipes[2][1]);

    fd = open("result.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0)
    {
        perror("open result.txt");
        return 1;
    }

    while ((n = read(pipes[2][0], buffer, sizeof(buffer))) > 0)
        write(fd, buffer, n);

    close(pipes[2][0]);
    close(fd);

    for (i = 0; i < 3; i++)
    {
        waitpid(pids[i], &status, 0);

        if (WIFEXITED(status))
            printf("Child %d (%s) exited with status %d\n", i + 1, stages[i][0], WEXITSTATUS(status));
        else
            printf("Child %d (%s) did not exit normally\n", i + 1, stages[i][0]);
    }

    // read the file back, show the first 5 lines and count all of them
    fd = open("result.txt", O_RDONLY);

    if (fd < 0)
    {
        perror("open result.txt");
        return 1;
    }

    n = read(fd, buffer, sizeof(buffer));
    close(fd);

    printf("\nFirst 5 lines of result.txt:\n");

    for (i = 0; i < n; i++)
    {
        if (shown < 5)
            putchar(buffer[i]);

        if (buffer[i] == '\n')
        {
            lines++;

            if (lines <= 5)
                shown = lines;
        }
    }

    printf("\nTotal lines: %d\n", lines);

    return 0;
}
