#pragma once
#include <stdint.h>

static inline void mmio_outl(uintptr_t addr, uint32_t d) {
    *(volatile uint32_t *) addr = d;
}

static inline uint32_t mmio_inl(uintptr_t addr) {
    return *(volatile uint32_t *) addr;
}
