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

int read(int fd, char *buf, size_t lim) {
    return (int) syscall3(0x03, fd, (uint32_t) buf, lim);
}

void _start(void *arg) {
    char buf[1];

    while (1) {
        char f = read(0, buf, sizeof(buf));
        write(0, buf, 1);
        f += '0';
        write(0, &f, 1);
        write(0, "\n", 1);
    }
}
