#pragma once
#include <stddef.h>

void *memset(void *p, int v, size_t sz);
void *memcpy(void *dst, const void *src, size_t len);

void fmtsiz(size_t sz, char *out);
