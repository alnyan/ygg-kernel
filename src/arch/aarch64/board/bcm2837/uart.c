#include "util.h"
#include "board.h"
#include "arch/aarch64/uart.h"

static inline int bcm2837_uart0_tx_busy(void) {
    return bcm2837_uart0->fr & (1 << 5);
}

void uart_send(uart_num_t uart, uint8_t c) {
    if (!uart) {
        while (bcm2837_uart0_tx_busy()) {}
        bcm2837_uart0->dr = c;
    }
}

void uart_config(uart_num_t uart, const uart_setup_t *setup) {
    if (uart) {
        return;
    }

    // Enable pins 14/15 for UART
    bcm2837_gpio->gppudclk0 = (1 << 14) | (1 << 15);
    delay(150);
    bcm2837_gpio->gppudclk0 = 0;

    // Clear pending interrupts of UART0
    bcm2837_uart0->icr = 0x7FF;
    // Set baud rate
    // Divider = UART_CLOCK / (16 * BAUD)
    // Fraction register = (Fraction part * 64) + 0.5
    // UART_CLOCK = 3000000, BAUD = 115200
    // Divider = 3000000 / (16 * 115200) = 1.627,
    //      Int part = 1
    //      Fract part = .627
    //      Fraction register = (.627 * 64) + 0.5 = 40.628 = 40
    uint32_t i = 3000000 / (16 * 115200);
    uint32_t f = ((((3000000 / 16) * 1000 /* Precision */ / setup->uart_baud) % 1000) * 64) / 1000;

    bcm2837_uart0->ibrd = i;
    bcm2837_uart0->fbrd = f;

    // Enable FIFO, word size 8 bit, 1 stop bit, parity none
    bcm2837_uart0->lcrh = (1 << 4) | (1 << 5) | (1 << 6);
    // Mask all interrupts
    bcm2837_uart0->imsc = setup->uart_intr;

    // Enable UART itself and Tx/Rx
    bcm2837_uart0->cr = (setup->uart_flags & 0x1) |
                        ((setup->uart_flags & 0x2) << 7) |
                        ((setup->uart_flags & 0x4) << 8);
}
