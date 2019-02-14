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
    return 0;
}
