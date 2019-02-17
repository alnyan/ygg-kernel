#pragma once
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#define debug(f, ...) debugf("[%s] " f, __func__, ##__VA_ARGS__)

void debug_xs(uint64_t v, char *res, const char *set);
void debug_ds(int64_t x, char *res, int s, int sz);
void debug_init(void);
void debugf(const char *fmt, ...);
void debugfv(const char *fmt, va_list args);

void debug_dump(const void *block, size_t count);
