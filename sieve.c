// Eratosthenes sieve using CSP

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define PIPE_READ  0
#define PIPE_WRITE 1

#define DEFAULT_RANGE 100

#define ERROR_FORK                                                             \
    fprintf(stderr, "%s:%d: fork() failed!: %s\n", __FILE__, __LINE__,         \
            strerror(errno));                                                  \
    return 2;
#define ERROR_PIPE                                                             \
    fprintf(stderr, "%s:%d: pipe() failed!: %s\n", __FILE__, __LINE__,         \
            strerror(errno));                                                  \
    return 3;
#define ERROR_WRITE                                                            \
    fprintf(stderr, "%s:%d: write() failed!: %s\n", __FILE__, __LINE__,        \
            strerror(errno));                                                  \
    return 4;

int main(int argc, char* argv[]) {
    long buf, prime, range;
    int in_pipe[2], out_pipe[2];

    if (argc == 1) {
        range = DEFAULT_RANGE;
    } else if (argc == 2) {
        range = strtol(argv[1], NULL, 10);
    } else {
        fprintf(stderr, "Usage: %s <RANGE>\n", argv[0]);
        return 1;
    }

    if (pipe(in_pipe) == -1) { ERROR_PIPE }

    // We want the unfiltered data to be fed in through a separate process or
    // the integer range will be limited by pipe's buffer size. Doing it this
    // way gives us "unbounded" range. i.e., bounded only by the width of
    // range's datatype.
    switch (fork()) {
        case -1: ERROR_FORK

        case 0:
            close(in_pipe[PIPE_READ]);

            for (long i = 2; i <= range; i++) {
                if (write(in_pipe[PIPE_WRITE], &i, sizeof(long)) == -1) {
                    ERROR_WRITE
                }
            }

            close(in_pipe[PIPE_WRITE]);
            return 0;

        default: break;
    }

    while (true) {
        close(in_pipe[PIPE_WRITE]);

        if (read(in_pipe[PIPE_READ], &prime, sizeof(long)) == 0) { return 0; }
        write(STDOUT_FILENO, &prime, sizeof(long));

        // Create a new output pipe before each fork
        if (pipe(out_pipe) == -1) { ERROR_PIPE };

        // Flush stdout before forking or bad libc buffering things will happen
        fflush(stdout);
        int pid = fork();

        switch (pid) {
            case -1: ERROR_FORK

            case 0:
                // Make out_pipe the new input pipe for when we fork again
                in_pipe[PIPE_READ]  = out_pipe[PIPE_READ];
                in_pipe[PIPE_WRITE] = out_pipe[PIPE_WRITE];
                break;

            default:
                close(in_pipe[PIPE_WRITE]);
                close(out_pipe[PIPE_READ]);

                long* buffer = malloc(100 * sizeof(long));
                size_t i     = 0;

                while (read(in_pipe[PIPE_READ], &buf, sizeof(long))) {
                    if (buf % prime == 0) { continue; }
                    buffer[i] = buf;
                    i++;
                    if (i == 100) {
                        write(out_pipe[PIPE_WRITE], buffer, sizeof(long) * 100);
                        i = 0;
                    }
                }

                write(out_pipe[PIPE_WRITE], buffer, sizeof(long) * i);

                free(buffer);
                close(in_pipe[PIPE_READ]);
                close(out_pipe[PIPE_WRITE]);

                // Wait for the child we just created or we'll end up with
                // orphan processes which are unkillable from the terminal
                waitpid(pid, 0, 0);
                return 0;
        }
    }
}
