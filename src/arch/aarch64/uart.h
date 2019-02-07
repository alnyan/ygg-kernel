#pragma once
#include <stdint.h>

// UART number, 0 will work for most boards
typedef uint32_t uart_num_t;

// UART initialization control struct
typedef struct {
    // Basic control flags:
    //  0 - enable UART
    //  1 - enable Rx
    //  2 - enable Tx
    uint32_t uart_flags;
    // Interrupt mask, board-specific
    uint32_t uart_intr;
    // UART baud rate
    uint32_t uart_baud;
    // TODO: add params like flow control/word size
} uart_setup_t;

// See actual implementations in board files
void uart_config(uart_num_t n, const uart_setup_t *setup);
void uart_default_config(uart_num_t n);
int uart_state(uart_num_t n);

void uart_send(uart_num_t n, uint8_t c);
uint8_t uart_recv(uart_num_t n);
