#pragma once
#include <stdint.h>

static inline void outb(uint16_t addr, uint8_t v) {
    asm volatile("outb %0, %1"::"a"(v), "Nd"(addr));
}

static inline uint8_t inb(uint16_t addr) {
	uint8_t v;
    asm volatile("inb %1, %0":"=a"(v):"Nd"(addr));
	return v;
}
