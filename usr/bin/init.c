#include <stdio.h>
#include <unistd.h>

void _start(void *arg) {
    char buf;
    while (1) {
        if (read(STDIN_FILENO, &buf, 1) != 1) {
            exit(1234);
        }

        write(STDOUT_FILENO, &buf, 1);
    }
    exit(0);
}
