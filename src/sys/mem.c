#include <stddef.h>

void *memset(void *block, int v, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ((char *) block)[i] = v;
    }
    return block;
}
