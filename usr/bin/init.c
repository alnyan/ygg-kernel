#include <stdio.h>
#include <unistd.h>

void _start(void *arg) {
    char buf[5];

    while (1) {
        if (read(0, buf, 5) != 5) {
            exit(1234);
        }

        write(0, "Test: ", 6);
        write(0, buf, 5);
        write(0, "\n", 1);
    }
    exit(0);
}
