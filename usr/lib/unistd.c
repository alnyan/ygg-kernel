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

int fork(void) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_FORK));
    return r;
}

int fexecve(const char *path, const char **argp, const char **envp) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NRX_FEXECVE),
            "b"(path),
            "c"(argp),
            "d"(envp));
    return r;
}

int execve(const char *path, const char **argp, const char **envp) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_EXECVE),
            "b"(path),
            "c"(argp),
            "d"(envp));
    return r;
}

pid_t getpid(void) {
    pid_t r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_GETPID));
    return r;
}

void nanosleep(const struct timespec *ts) {
    asm volatile ("int $0x80"::
            "a"(SYSCALL_NR_NANOSLEEP),
            "b"(ts));
}

void sleep(unsigned int sec) {
    struct timespec ts = { sec, 0 };
    nanosleep(&ts);
}

void puts(const char *s) {
    size_t l = strlen(s);
    write(STDOUT_FILENO, s, l);
}

void putc(char c) {
    write(STDOUT_FILENO, &c, 1);
}
