#pragma once
#include <stdint.h>
#include <stddef.h>
// For struct timespec
#include <uapi/time.h>

typedef int ssize_t;
typedef int pid_t;

struct dirent {
    uint32_t d_ino;
    uint32_t d_off;
    uint16_t d_reclen;
    uint8_t d_type;
    char d_name[256];
};

#define STDOUT_FILENO       0
#define STDIN_FILENO        1
#define STDERR_FILENO       2

#define DT_CHR              2
#define DT_DIR              4
#define DT_BLK              6
#define DT_REG              8

#define O_RDONLY            (1 << 0)
#define O_WRONLY            (1 << 1)
#define O_RDWR              (O_RDONLY | O_WRONLY)
#define O_DIRECTORY         (1 << 21)

ssize_t read(int fd, void *data, size_t len);
ssize_t write(int fd, const void *data, size_t len);

int open(const char *path, int flags, uint32_t mode);
void close(int fd);
// TODO: proper interface for these
int opendir(const char *path);
int readdir(int fd, struct dirent *ent);
#define closedir close

void putc(char c);
void puts(const char *s);

int fork(void);
void exit(int r);
int execve(const char *path, const char **argp, const char **envp);
int fexecve(const char *path, const char **argp, const char **envp);

pid_t getpid(void);
int kill(pid_t pid, int sig);

int nanosleep(const struct timespec *ts);
int sleep(unsigned int sec);
