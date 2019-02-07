#include "board.h"
#include "arch/aarch64/mmio.h"
#include "arch/aarch64/uart.h"

static inline int bcm2837_uart0_tx_busy(void) {
    return mmio_inl(BCM2837_UART0_BASE + BCM2837_UART_FR) & (1 << 5);
}

void uart_send(uart_num_t uart, uint8_t c) {
    if (!uart) {
        while (bcm2837_uart0_tx_busy()) {}
        mmio_outl(BCM2837_UART0_BASE + BCM2837_UART_DR, c);
    }
}
