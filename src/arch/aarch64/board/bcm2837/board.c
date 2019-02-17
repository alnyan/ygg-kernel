#include "board.h"
#include "arch/aarch64/mmio.h"
#include "arch/aarch64/uart.h"
#include "sys/debug.h"

void hw_early_init(void) {
    // Setup UART0
    // Disable UART0
    bcm2837_uart0->cr = 0;
    bcm2837_gpio_init();

    uart_default_config(0);
}

void hw_init(void) {
    bcm283xsp804->control &= ~(1 << 7);
    bcm2835_irq_regs->irq_basic_enable |= 1;
    bcm283xsp804->load = 1000;
    bcm283xsp804->control = (1 << 7) | (2 << 2) | (1 << 1) | (1 << 5);
}
