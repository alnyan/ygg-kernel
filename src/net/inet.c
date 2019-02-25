#include "inet.h"
#include "sys/debug.h"
#include "sys/string.h"

void inet_ntoa(char *out, inaddr_t a) {
    size_t l = 0;
    for (int i = 0; i < 4; ++i) {
        debug_ds((a >> (i * 8)) & 0xFF, &out[l], 0, 0);
        l = strlen(out);
        if (i != 3) {
            out[l++] = '.';
            out[l] = 0;
        }
    }
}
