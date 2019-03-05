#include <uapi/syscall.h>
#include "unistd.h"
#include "errno.h"

#define __set_errno(x) if (x < 0) { errno = -x; return -1; } else { errno = 0; return x; }

int readdir(int dir, struct dirent *ent) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_READ),
            "b"(dir),
            "c"(ent),
            "d"(sizeof(struct dirent)));
    __set_errno(r);
}

ssize_t read(int fd, void *data, size_t len) {
    ssize_t r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_READ),
            "b"(fd),
            "c"(data),
            "d"(len));
    __set_errno(r);
}

ssize_t write(int fd, const void *data, size_t len) {
    ssize_t r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_WRITE),
            "b"(fd),
            "c"(data),
            "d"(len));
    __set_errno(r);
}

int open(const char *path, int flags, uint32_t mode) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_OPEN),
            "b"(path),
            "c"(flags),
            "d"(mode));
    __set_errno(r);
}

int opendir(const char *path) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_OPEN),
            "b"(path),
            "c"(O_DIRECTORY),
            "d"(0));
    __set_errno(r);
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
    __set_errno(r);
}

int fexecve(const char *path, const char **argp, const char **envp) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NRX_FEXECVE),
            "b"(path),
            "c"(argp),
            "d"(envp));
    __set_errno(r);
}

int execve(const char *path, const char **argp, const char **envp) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_EXECVE),
            "b"(path),
            "c"(argp),
            "d"(envp));
    __set_errno(r);
}

pid_t getpid(void) {
    pid_t r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_GETPID));
    return r;
}

int nanosleep(const struct timespec *ts) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_NANOSLEEP),
            "b"(ts));
    __set_errno(r);
}

int kill(pid_t pid, int sig) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_KILL),
            "b"(pid),
            "c"(sig));
    __set_errno(r);
}

int waitpid(pid_t pid, int *wstatus, int options) {
    int r;
    asm volatile ("int $0x80":
            "=a"(r):
            "a"(SYSCALL_NR_WAITPID),
            "b"(pid),
            "c"(wstatus),
            "d"(options));
    __set_errno(r);
}
