#include <stdio.h>
#include <unistd.h>

void _start(void) {
    int *v = (int *) 0x12345678;
    *v = 1234;

    while (1) {
        printf("Hello from #%d!\n", getpid());
        sleep(3);
    }
    exit(0);
}
