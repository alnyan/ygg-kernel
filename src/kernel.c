#include "arch/aarch64/mmio.h"
#include "arch/aarch64/uart.h"
#include "arch/aarch64/board/bcm2837/board.h"
#include "sys/debug.h"

int hw_init(void) {
    return bcm2837_init_hw();
}

void kernel_main(void) {
    hw_init();

    debug_init();

    debug("AAAA\n");

    while (1) {
        asm volatile("wfe");
    }
}
