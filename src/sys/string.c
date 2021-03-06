#include "string.h"

size_t strlen(const char *s) {
    size_t l = 0;
    while (*s++) {
        ++l;
    }
    return l;
}

int strncmp(const char *a, const char *b, size_t l) {
    size_t p;
    for (p = 0; p < l && *a && *b; ++a, ++b, ++p) {
        if (*a != *b) {
            return 1;
        }
    }
    if (*a != *b && p != l) {
        return 1;
    }
    return 0;
}

int strcmp(const char *a, const char *b) {
    return strncmp(a, b, 0xFFFFFFFF);
}

size_t strncmn(const char *a, const char *b, size_t n) {
    size_t cmn = 0;
    for (size_t i = 0; i < n && a[i] && b[i]; ++i, ++cmn) {
        if (a[i] != b[i]) {
            break;
        }
    }
    return cmn;
}

char *strcpy(char *dst, const char *src) {
    size_t i;
    for (i = 0; src[i]; ++i) {
        dst[i] = src[i];
    }
    dst[i] = 0;
    return dst;
}

char *strncpy(char *dst, const char *src, size_t l) {
    size_t i;
    for (i = 0; src[i] && i < l; ++i) {
        dst[i] = src[i];
    }
    if (i != l) {
        dst[i] = 0;
    }
    return dst;
}

char *strchr(const char *s, char c) {
    for (size_t i = 0; s[i]; ++i) {
        if (s[i] == c) {
            return (char *) &s[i];
        }
    }
    return NULL;
}

char *strchrnul(const char *s, char c) {
    for (size_t i = 0;; ++i) {
        if (!s[i]) {
            return (char *) &s[i];
        }
        if (s[i] == c) {
            return (char *) &s[i];
        }
    }
}
