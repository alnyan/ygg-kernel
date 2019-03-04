#include "string.h"

size_t strlen(const char *s) {
    size_t l = 0;
    for (; *s; ++s, ++l) {}
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
    while (1) {
        if (*a != *b) {
            return 1;
        }

        if (*a == 0) {
            return 0;
        }

        ++a;
        ++b;
    }
}

char *strchr(const char *s, char c) {
    for (char *p = (char *) s; *p; ++p) {
        if (*p == c) {
            return p;
        }
    }
    return NULL;
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

void *memset(void *buf, int c, size_t l) {
    for (size_t i = 0; i < l; ++i) {
        ((char *) buf)[i] = (char) c;
    }
    return buf;
}
