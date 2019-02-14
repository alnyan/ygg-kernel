#include <stddef.h>

void *memset(void *block, int v, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ((char *) block)[i] = v;
    }
    return block;
}

void *memcpy(void *dst, const void *src, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ((char *) dst)[i] = ((const char *) src)[i];
    }
    return dst;
}
