#pragma once
#include <stdint.h>

//// GPIO
#define BCM2837_GPIO_BASE       0x3F200000
#define BCM2837_GPIO_GPPUD      0x94
#define BCM2837_GPIO_GPPUDCLK0  0x98

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

int bcm2837_init_hw(void);
