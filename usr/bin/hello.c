#include <stdio.h>
#include <unistd.h>

int main(void) {
    while (1) {
        printf("Hello from #%d!\n", getpid());
        sleep(3);
    }
    exit(0);
}
