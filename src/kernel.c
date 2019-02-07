#include "sys/debug.h"
#include "util.h"

// Ugly right now, will move to "hw" header
#if defined(ARCH_AARCH64) && defined(BOARD_BCM2837)
#include "arch/aarch64/board/bcm2837/board.h"
#endif

int hw_init(void) {
#ifdef ARCH_AARCH64
    return bcm2837_init_hw();
#else
    return 0;
#endif
}

void kernel_main(void) {
    hw_init();

    debug_init();
    debug("AAAA\n");

    while (1) {
        __idle();
    }
}
