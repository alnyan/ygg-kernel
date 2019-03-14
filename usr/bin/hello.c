#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static void handle_usr1(int signum) {
    printf("I'm USR1 handler in %d, hello\n", getpid());
}

int main(void) {
    // signal(SIGUSR1, handle_usr1);
    printf("Hello\n");

    // for (int i = 0; i < 10; ++i) {
    //     printf("Hello from #%d!\n", getpid());
    //     if (sleep(3) != 0) {
    //         printf("Waking up from aborted sleep\n");
    //     }
    // }
    exit(0);
}
