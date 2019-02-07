#include "arch/aarch64/mmio.h"
#include "arch/aarch64/board/bcm2837.h"

int hw_init(void) {
    return bcm2837_init_hw();
}

void kernel_main(void) {
    hw_init();

    while (1) {
    }
}
