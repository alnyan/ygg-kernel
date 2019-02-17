#include "unistd.h"
#include "string.h"
#include "syscall.h"

ssize_t write(int fd, const void *data, size_t len) {
    ssize_t r;
#if defined(ARCH_X86)
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_WRITE),
            "b"(fd),
            "c"(data),
            "d"(len));
#endif
    return r;
}

void exit(int r) {
#if defined(ARCH_X86)
    asm volatile ("int $0x80"::
            "a"(SYSCALL_NR_EXIT),
            "b"(r));
#endif
    while (1);
}

void puts(const char *s) {
    size_t l = strlen(s);
    write(STDOUT_FILENO, s, l);
}

void putc(char c) {
    write(STDOUT_FILENO, &c, 1);
}
