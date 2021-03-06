#pragma once
#include <stdarg.h>
#include <stddef.h>

// Not like libc, but works for us
int printf(const char *fmt, ...);
int snprintf(char *buf, size_t len, const char *fmt, ...);
int dprintf(int fd, const char *fmt, ...);

int vsnprintf(char *dst, size_t len, const char *fmt, va_list args);
int vprintf(const char *fmt, va_list args);
int vdprintf(int fd, const char *fmt, va_list args);
