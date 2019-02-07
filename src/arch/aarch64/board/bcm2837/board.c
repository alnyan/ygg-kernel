#include "board.h"
#include "arch/aarch64/mmio.h"

int bcm2837_init_hw(void) {
    // Setup UART0
    // Disable UART0
    bcm2837_uart0->cr = 0;

    // Setup GPIO14/GPIO15
    bcm2837_gpio->gppud = 0;
    delay(150);

    return 0;
}
