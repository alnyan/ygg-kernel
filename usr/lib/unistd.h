#pragma once
#include <stdint.h>
#include <stddef.h>

typedef int ssize_t;

#define STDOUT_FILENO       0
#define STDIN_FILENO        1
#define STDERR_FILENO       2

#define O_RDONLY            (1 << 0)
#define O_WRONLY            (1 << 1)
#define O_RDWR              (O_RDONLY | O_WRONLY)

ssize_t read(int fd, void *data, size_t len);
ssize_t write(int fd, const void *data, size_t len);

int open(const char *path, int flags, uint32_t mode);

void putc(char c);
void puts(const char *s);

void exit(int r);
