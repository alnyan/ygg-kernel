#pragma once
#include <stdint.h>

//// GPIO
#define BCM2837_GPIO_BASE       0x3F200000
#define BCM2837_GPIO_GPPUD      0x94
#define BCM2837_GPIO_GPPUDCLK0  0x98

struct bcm2837_gpio {
    uint32_t gppsel0;
    uint32_t gppsel1;
    uint32_t gppsel2;
    uint32_t gppsel3;
    uint32_t gppsel4;
    uint32_t gppsel5;
    uint32_t res0;
    // Set GPIOn value to 1
    uint32_t gpset0;
    uint32_t gpset1;
    uint32_t res1;
    // Set GPIOn value to 0
    uint32_t gpclr0;
    uint32_t gpclr1;
    uint32_t res2;
    uint32_t gplev0;
    uint32_t gplev1;
    uint32_t pad[22];
    uint32_t gppud;
    uint32_t gppudclk0;
};

//// UART
#define BCM2837_UART0_BASE      0x3F201000
#define BCM2837_UART_DR         0x00
#define BCM2837_UART_RSRECR     0x04
#define BCM2837_UART_FR         0x18
#define BCM2837_UART_ILPR       0x20
#define BCM2837_UART_IBRD       0x24
#define BCM2837_UART_FBRD       0x28
#define BCM2837_UART_LCRH       0x2C
#define BCM2837_UART_CR         0x30
#define BCM2837_UART_IFLS       0x34
#define BCM2837_UART_IMSC       0x38
#define BCM2837_UART_RIS        0x3C
#define BCM2837_UART_MIS        0x40
#define BCM2837_UART_ICR        0x44
#define BCM2837_UART_DMACR      0x48
#define BCM2837_UART_ITCR       0x80
#define BCM2837_UART_ITIP       0x84
#define BCM2837_UART_ITOP       0x88
#define BCM2837_UART_TDR        0x8C

struct bcm2837_uart {
    uint32_t dr;
    uint32_t rsrecr;
    uint32_t pad0[4];
    uint32_t fr;
    uint32_t pad1;
    uint32_t ilpr;
    // Baud control
    uint32_t ibrd;
    uint32_t fbrd;
    // Transport and general control
    uint32_t lcrh;
    uint32_t cr;
    // IT control
    uint32_t ifls;
    uint32_t imsc;
    uint32_t ris;
    uint32_t mis;
    uint32_t icr;
    uint32_t dmacr;
    // Test control regs, unused
    uint32_t pad2[13];
    uint32_t itcr;
    uint32_t itip;
    uint32_t itop;
    uint32_t tdr;
};

//// ARM timer
#define BCM283xS804_BASE        0x3F00B400

struct bcm283xsp804 {
    uint32_t load;
    uint32_t value;
    uint32_t control;
    uint32_t irq_clr;
    uint32_t irq_raw;
    uint32_t irq_mask;
    uint32_t reload;
    uint32_t prediv;
    uint32_t freecntr;
};

//// IRQ Controller
#define BCM2835_IRQ_REGS_BASE   0x3F00B200

struct bcm2835_irq_regs {
    uint32_t irq_basic_pending;
    uint32_t irq_pending1;
    uint32_t irq_pending2;
    uint32_t fiq_ctl;
    uint32_t irq_enable1;
    uint32_t irq_enable2;
    uint32_t irq_basic_enable;
    uint32_t irq_disable1;
    uint32_t irq_disable2;
    uint32_t irq_basic_disable;
};

// MMIO
static volatile struct bcm2835_irq_regs *bcm2835_irq_regs = (struct bcm2835_irq_regs *) BCM2835_IRQ_REGS_BASE;
static volatile struct bcm283xsp804 *bcm283xsp804 = (struct bcm283xsp804 *) BCM283xS804_BASE;
static volatile struct bcm2837_gpio *bcm2837_gpio = (struct bcm2837_gpio *) BCM2837_GPIO_BASE;
static volatile struct bcm2837_uart *bcm2837_uart0 = (struct bcm2837_uart *) BCM2837_UART0_BASE;

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count) {
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

void bcm2837_gpio_init(void);
