#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {

    ssize_t rdstat;
    long buf;

    while (rdstat = read(STDIN_FILENO, &buf, sizeof(long))) {
        switch (rdstat) {
            case -1: return 2;
            case 0: return 0;
            default: printf("%ld\n", buf); break;
        }
    }

    return 0;
}
