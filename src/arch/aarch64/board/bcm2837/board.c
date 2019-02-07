#include "board.h"
#include "arch/aarch64/mmio.h"

void hw_init(void) {
    // Setup UART0
    // Disable UART0
    bcm2837_uart0->cr = 0;

    bcm2837_gpio_init();
}
