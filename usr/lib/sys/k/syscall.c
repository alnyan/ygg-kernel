#include <uapi/syscall.h>
#include "unistd.h"

int readdir(int dir, struct dirent *ent) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_READ),
            "b"(dir),
            "c"(ent),
            "d"(sizeof(struct dirent)));
    return r;
}

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

int opendir(const char *path) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_OPEN),
            "b"(path),
            "c"(O_DIRECTORY),
            "d"(0));
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
