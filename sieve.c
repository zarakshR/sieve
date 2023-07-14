// Eratosthenes sieve using CSP

#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

#define DEFAULT_STOP 100

#define ERROR_FORK                                                             \
    fprintf(stderr, "fork() failed!\n");                                       \
    return 2;
#define ERROR_PIPE                                                             \
    fprintf(stderr, "pipe() failed!\n");                                       \
    return 3;
#define ERROR_WRITE                                                            \
    fprintf(stderr, "write() failed!\n");                                      \
    return 4;

int main(int argc, char* argv[]) {
    int buf, prime, range;
    int in_pipe[2], out_pipe[2];

    if(argc == 2) {
        range = strtod(argv[1], NULL);
    } else {
        range = DEFAULT_STOP;
    }

    if (pipe(in_pipe) == -1) { ERROR_PIPE }

    switch (fork()) {
        case -1: ERROR_FORK

        case 0:
            // Initialize the pipe with the entire unfiltered data
            // We want the initialization to be done on a separate process or
            // the integer range will be limited by pipe's buffer size
            close(in_pipe[PIPE_READ]);

            for (int i = 2; i <= range; i++) {
                if (write(in_pipe[PIPE_WRITE], &i, sizeof(int)) == -1) {
                    ERROR_WRITE
                }
            }

            close(in_pipe[PIPE_WRITE]);
            return 0;

        default: break;
    }

    while (true) {
        close(in_pipe[PIPE_WRITE]);

        if(read(in_pipe[PIPE_READ], &prime, sizeof(int)) == 0) { return 0; }
        printf("PID: %d\tPGID:%d\tPrime: %d\n", getpid(), getpgid(0), prime);

        // Create a new output pipe before each fork
        if (pipe(out_pipe) == -1) { ERROR_PIPE };

        // Flush stdout before forking or bad libc buffering things will happen
        fflush(stdout);
        int pid = fork();

        switch (pid) {
            case -1: ERROR_FORK

            case 0:
                // Make out_pipe the new input pipe for when we fork again
                in_pipe[PIPE_READ] = out_pipe[PIPE_READ];
                in_pipe[PIPE_WRITE] = out_pipe[PIPE_WRITE];
                break;
            default:
                close(in_pipe[PIPE_WRITE]);
                close(out_pipe[PIPE_READ]);

                while(read(in_pipe[PIPE_READ], &buf, sizeof(int))) {
                    if(buf % prime != 0) {
                        if(write(out_pipe[PIPE_WRITE], &buf, sizeof(int)) == -1) { fprintf(stderr, "write() failed!\n"); return 4; };
                    }
                }

                close(in_pipe[PIPE_READ]);
                close(out_pipe[PIPE_WRITE]);

                // Wait for the child we just created or we'll end up with
                // orphan processes which are unkillable from the terminal
                waitpid(pid, 0, 0);
                return 0;
        }
    }
}
