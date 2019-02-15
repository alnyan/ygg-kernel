#include "board.h"
#include "arch/aarch64/mmio.h"
#include "sys/debug.h"

void hw_init(void) {
    // Setup UART0
    // Disable UART0
    bcm2837_uart0->cr = 0;

    bcm2837_gpio_init();

    bcm283xsp804->control &= ~(1 << 7);
    bcm2835_irq_regs->irq_basic_enable |= 1;
    bcm283xsp804->reload = 1000;
    bcm283xsp804->prediv = 999;
    bcm283xsp804->control = (1 << 7) | (1 << 9) | (2 << 2) | (1 << 1) | (1 << 5) | (0 << 16);

    asm volatile ("msr daifclr, #0x2");

    while (1) {
        debug("%u\n", bcm283xsp804->freecntr);
        for (int i = 0; i < 10000000; ++i);
    }
}
