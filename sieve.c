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

#define TX_RX_BUF_SIZE 100

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
#define ERROR_READ                                                             \
    fprintf(stderr, "%s:%d: read() failed!: %s\n", __FILE__, __LINE__,         \
            strerror(errno));                                                  \
    return 5;

int main(int argc, char* argv[]) {
    unsigned long prime, range;
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

            for (unsigned long i = 2; i <= range; i++) {
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
        if (write(STDOUT_FILENO, &prime, sizeof(long)) == -1) { ERROR_WRITE }

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

                long* rx_buffer = malloc(sizeof(long) * TX_RX_BUF_SIZE);
                long* tx_buffer = malloc(sizeof(long) * TX_RX_BUF_SIZE);

                ssize_t nr = 0; // bytes read
                ssize_t nw = 0; // bytes written

                ssize_t total_nr = 0; // total bytes read
                ssize_t total_nw = 0; // total bytes written

                while (true) {

                    ssize_t rxi = 0; // rxbuffer index
                    ssize_t txi = 0; // txbuffer index

                    nr = read(in_pipe[PIPE_READ], rx_buffer,
                              sizeof(long) * TX_RX_BUF_SIZE);

                    switch (nr) {
                        case -1: ERROR_READ
                        case 0: goto exit;
                        default:
                            for (; rxi < (nr / sizeof(long)); rxi++) {
                                if (rx_buffer[rxi] % prime != 0) {
                                    tx_buffer[txi] = rx_buffer[rxi];
                                    txi++;
                                }
                            }
                    }
                    nw = write(out_pipe[PIPE_WRITE], tx_buffer,
                               sizeof(long) * txi);
                }

            exit:
                free(rx_buffer);
                free(tx_buffer);

                close(in_pipe[PIPE_READ]);
                close(out_pipe[PIPE_WRITE]);

                // Wait for the child we just created or we'll end up with
                // orphan processes which are unkillable from the terminal
                waitpid(pid, 0, 0);
                return 0;
        }
    }
}
