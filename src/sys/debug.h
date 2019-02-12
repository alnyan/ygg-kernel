#pragma once
#include <stdarg.h>
#include <stdint.h>
#define debug(f, ...) debugf("[%s] " f, __FUNCTION__, ##__VA_ARGS__)

void debug_xs(uint64_t v, char *res, const char *set);
void debug_init(void);
void debugf(const char *fmt, ...);
void debugfv(const char *fmt, va_list args);
