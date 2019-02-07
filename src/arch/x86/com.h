#pragma once
#include <stdint.h>

#define X86_COM0    0x3F8

void com_init(uint16_t port);
void com_send(uint16_t port, char c);
char com_recv(uint16_t port);
