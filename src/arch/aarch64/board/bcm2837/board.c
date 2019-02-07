#include "board.h"
#include "arch/aarch64/mmio.h"

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count) {
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

int bcm2837_init_hw(void) {
    // Setup UART0
    // Disable UART0
    mmio_outl(BCM2837_UART0_BASE + BCM2837_UART_CR, 0x00000000);

    // Setup GPIO14/GPIO15
    mmio_outl(BCM2837_GPIO_BASE + BCM2837_GPIO_GPPUD, 0x00000000);
    delay(150);
    mmio_outl(BCM2837_GPIO_BASE + BCM2837_GPIO_GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    mmio_outl(BCM2837_GPIO_BASE + BCM2837_GPIO_GPPUDCLK0, 0x00000000);

    // Clear pending interrupts of UART0
    mmio_outl(BCM2837_UART0_BASE + BCM2837_UART_ICR, 0x7FF);
    // Set baud rate
    // Divider = UART_CLOCK / (16 * BAUD)
    // Fraction register = (Fraction part * 64) + 0.5
    // UART_CLOCK = 3000000, BAUD = 115200
    // Divider = 3000000 / (16 * 115200) = 1.627,
    //      Int part = 1
    //      Fract part = .627
    //      Fraction register = (.627 * 64) + 0.5 = 40.628 = 40
    mmio_outl(BCM2837_UART0_BASE + BCM2837_UART_IBRD, 1);
    mmio_outl(BCM2837_UART0_BASE + BCM2837_UART_FBRD, 40);

    // Enable FIFO, word size 8 bit, 1 stop bit, parity none
    mmio_outl(BCM2837_UART0_BASE + BCM2837_UART_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
    // Mask all interrupts
    mmio_outl(BCM2837_UART0_BASE + BCM2837_UART_IMSC, (1 << 1) |
                                                       (1 << 4) |
                                                       (1 << 5) |
                                                       (1 << 6) |
                                                       (1 << 7) |
                                                       (1 << 8) |
                                                       (1 << 9) |
                                                       (1 << 10));

    // Enable UART itself and Tx/Rx
    mmio_outl(BCM2837_UART0_BASE + BCM2837_UART_CR, (1 << 0) | (1 << 8) | (1 << 9));

    return 0;
}
