#pragma once
#include <stddef.h>

typedef int ssize_t;

#define STDOUT_FILENO       0
#define STDIN_FILENO        1
#define STDERR_FILENO       2

ssize_t write(int fd, const void *data, size_t len);

void putc(char c);
void puts(const char *s);

void exit(int r);
