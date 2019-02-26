#pragma once
#include <stddef.h>

size_t strlen(const char *s);
int strncmp(const char *a, const char *b, size_t n);
int strcmp(const char *a, const char *b);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t lim);

char *strchr(const char *s, char c);
char *strchrnul(const char *s, char c);

// Custom func - returns count of first symbols that are the same in both input strings
size_t strncmn(const char *a, const char *b, size_t n);
