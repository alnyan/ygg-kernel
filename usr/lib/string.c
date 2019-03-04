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
