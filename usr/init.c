#include <stddef.h>
#include <stdint.h>

static uint32_t syscall3(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    uint32_t res;
    asm volatile("int $0x80":"=a"(res):"a"(eax), "b"(ebx), "c"(ecx), "d"(edx));
    return res;
}

int write(int fd, const char *buf, size_t lim) {
    return (int) syscall3(0x04, fd, (uint32_t) buf, lim);
}

static char some_var[3];
static int cntr = 0;

void _start(void *arg) {
    some_var[2] = 0;
    some_var[1] = '\n';

    while (1) {
        ++cntr;
        some_var[0] = (cntr % ('z' - 'a')) + 'a';

        write(0, some_var, 2);

        for (int i = 0; i < 0x400000; ++i) ;
    }
}
