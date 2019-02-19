#include "string.h"

size_t strlen(const char *s) {
    size_t l = 0;
    for (; *s; ++s, ++l) {}
    return l;
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
