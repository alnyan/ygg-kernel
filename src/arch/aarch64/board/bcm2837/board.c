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
    bcm283xsp804->load = 1000;
    bcm283xsp804->control = (1 << 7) | (2 << 2) | (1 << 1) | (1 << 5);

    asm volatile ("msr daifclr, #0x2");
}
