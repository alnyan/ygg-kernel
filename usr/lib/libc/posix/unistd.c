#include "unistd.h"
#include "string.h"
#include <uapi/syscall.h>

int sleep(unsigned int sec) {
    struct timespec ts = { sec, 0 };
    return nanosleep(&ts);
}

void puts(const char *s) {
    size_t l = strlen(s);
    write(STDOUT_FILENO, s, l);
    putc('\n');
}

void putc(char c) {
    write(STDOUT_FILENO, &c, 1);
}
