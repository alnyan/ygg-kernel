#pragma once
#include <stdarg.h>
#define debug(f, ...) debugf(f, ##__VA_ARGS__)

void debugf(const char *fmt, ...);
void debugfv(const char *fmt, va_list args);
