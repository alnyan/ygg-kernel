#pragma once
#include <stddef.h>

size_t strlen(const char *);
int strncmp(const char *a, const char *b, size_t l);
int strcmp(const char *a, const char *b);
char *strchr(const char *s, char c);

char *strncpy(char *dst, const char *src, size_t l);
char *strcpy(char *dst, const char *src);

void *memset(void *buf, int v, size_t sz);
