#pragma once
#include <stdarg.h>
#include <stddef.h>

// Not like libc, but works for us
void printf(const char *fmt, ...);
int vsnprintf(char *dst, size_t len, const char *fmt, va_list args);
