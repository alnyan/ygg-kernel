#include "sys/debug.h"
#include "arch/hw.h"
#include "util.h"

void kernel_main(void) {
    hw_init();

    debug_init();
    debug("AAAA\n");

    // Both should be '-1'
    debug("%d %ld\n", 0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    // And these shouldn't
    debug("%u %lu\n", 0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    // Should be okay
    debug("%d %ld\n", 0x7FFFFFFF, 0x7FFFFFFFFFFFFFFF);
    debug("%d %ld\n", -0x7FFFFFFF, -0x7FFFFFFFFFFFFFFF);


    // Some hex numbers
    debug("0x%x 0x%X\n", 0x1234BADB, 0x4321BDAB);
    debug("0x%lx 0x%lX\n", 0x12345678BADB002, 0x200BDAB87654321);

    /*__enable_irq();*/
    irq_enable();

    while (1) {
        __idle();
    }
}
