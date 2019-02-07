#pragma once
#include <stdarg.h>
#define debug(f, ...) debugf("[%s] " f, __FUNCTION__, ##__VA_ARGS__)

void debug_init(void);
void debugf(const char *fmt, ...);
void debugfv(const char *fmt, va_list args);
