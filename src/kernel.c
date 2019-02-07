#include "sys/debug.h"
#include "arch/hw.h"
#include "util.h"

void kernel_main(void) {
    hw_init();

    debug_init();
    debug("AAAA\n");

    while (1) {
        __idle();
    }
}
