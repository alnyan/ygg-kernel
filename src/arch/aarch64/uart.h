#pragma once

typedef uint32_t uart_num_t;

void uart_send(uart_num_t n, uint8_t c);
uint8_t uart_recv(uart_num_t n);
