#include "stdio.h"
#include "unistd.h"
#include "string.h"

#ifndef PRINTF_BUFFER_SIZE
#define PRINTF_BUFFER_SIZE      512
#endif

int printf(const char *fmt, ...) {
    char buf[PRINTF_BUFFER_SIZE];

    va_list args;
    va_start(args, fmt);
    int r = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    return write(STDOUT_FILENO, buf, r);
}

int dprintf(int fd, const char *fmt, ...) {
    char buf[PRINTF_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    int r = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return write(fd, buf, r);
}

int snprintf(char *buf, size_t len, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = vsnprintf(buf, len, fmt, args);
    va_end(args);
    return r;
}

int vprintf(const char *fmt, va_list args) {
    char buf[PRINTF_BUFFER_SIZE];
    int r = vsnprintf(buf, sizeof(buf), fmt, args);
    return write(STDOUT_FILENO, buf, r);
}

int vdprintf(int fd, const char *fmt, va_list args) {
    char buf[PRINTF_BUFFER_SIZE];
    int r = vsnprintf(buf, sizeof(buf), fmt, args);
    return write(fd, buf, r);
}
