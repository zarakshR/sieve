#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char* argv[], char* envp[]) {

    int fd;
    ssize_t rdstat;
    long buf;

    if((fd = open("nums1k.bin", O_RDONLY)) == -1) {
        return 1;
    }

    while(rdstat = read(fd, &buf, sizeof(long))) {
        switch(rdstat) {
            case -1: return 2;
            case 0: return 0;
            default:
                printf("%ld\n", buf);
                break;
        }
    }

    return 0;
}

