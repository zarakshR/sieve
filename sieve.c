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

int main(int argc, char* argv[]) {
    int buf, prime, range;
    int in_pipe[2], out_pipe[2];

    if(argc == 2) {
        range = strtod(argv[1], NULL);
    } else {
        range = DEFAULT_STOP;
    }

    // This is the maximum no of ints that can fit in a pipe
    if(range > 16384) { fprintf(stderr, "Range too big!\n"); return 5; }

    if(pipe(in_pipe) == -1) { fprintf(stderr, "pipe() failed!\n"); return 3; };

    // Initialize the pipe with the entire unfiltered data
    for(int i = 2; i <= range; i++) {
        if(write(in_pipe[PIPE_WRITE], &i, sizeof(int)) == -1) { fprintf(stderr, "write() failed!\n"); return 4; };
    }

    while(true) {

        close(in_pipe[PIPE_WRITE]);

        if(read(in_pipe[PIPE_READ], &prime, sizeof(int)) == 0) { return 0; }
        printf("PID: %d\tPGID:%d\tPrime: %d\n", getpid(), getpgid(0), prime);

        // Create a new output pipe before each fork
        if(pipe(out_pipe) == -1) { fprintf(stderr, "pipe() failed!\n"); return 3; };

        // Flush stdout before forking or bad libc buffering things will happen
        switch(fflush(stdout), fork()) {
            case -1: fprintf(stderr, "fork() failed!\n"); return 2;
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
                wait(0);
                return 0;
        }
    }
}
