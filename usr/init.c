#include <stddef.h>
#include <stdint.h>

static uint16_t *s_vidmem;

int write(int fd, const char *buf, size_t lim) {
    int res;
    asm volatile("int $0x80":"=a"(res));
    return res;
}

void _start(void *arg) {
    s_vidmem = (uint16_t *) arg;
    int v = 0;

    while (1) {
        v = !v;

        s_vidmem[0] = v * (0x0700 | (write(0, 0, 0) % ('z' - 'a') + 'a'));

        for (int i = 0; i < 0x4000000; ++i) ;
    }
}
