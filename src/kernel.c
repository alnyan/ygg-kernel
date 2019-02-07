#include "arch/aarch64/mmio.h"
#include "arch/aarch64/uart.h"
#include "arch/aarch64/board/bcm2837/board.h"

int hw_init(void) {
    return bcm2837_init_hw();
}

void kernel_main(void) {
    hw_init();

    uart_setup_t uartc = {
        .uart_baud = 115200,
        .uart_intr = 0x7FF,
        .uart_flags = (1 << 0) | (1 << 1) | (1 << 2)
    };
    uart_config(0, &uartc);

    uart_send(0, 'A');
    uart_send(0, '!');

    while (1) {
    }
}
