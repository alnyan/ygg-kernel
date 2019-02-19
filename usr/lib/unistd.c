#include "unistd.h"
#include "string.h"
#include "syscall.h"

ssize_t read(int fd, void *data, size_t len) {
    ssize_t r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_READ),
            "b"(fd),
            "c"(data),
            "d"(len));
    return r;
}

ssize_t write(int fd, const void *data, size_t len) {
    ssize_t r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_WRITE),
            "b"(fd),
            "c"(data),
            "d"(len));
    return r;
}

int open(const char *path, int flags, uint32_t mode) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_OPEN),
            "b"(path),
            "c"(flags),
            "d"(mode));
    return r;
}

void close(int fd) {
    asm volatile ("int $0x80"::"a"(SYSCALL_NR_CLOSE), "b"(fd));
}

void exit(int r) {
    asm volatile ("int $0x80"::
            "a"(SYSCALL_NR_EXIT),
            "b"(r));
    while (1);
}

void puts(const char *s) {
    size_t l = strlen(s);
    write(STDOUT_FILENO, s, l);
}

void putc(char c) {
    write(STDOUT_FILENO, &c, 1);
}
