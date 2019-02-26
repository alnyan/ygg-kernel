#include <stdio.h>
#include <unistd.h>

void _start(void) {
    while (1) {
        printf("Hello!\n");
        sleep(3);
    }
    exit(0);
}
