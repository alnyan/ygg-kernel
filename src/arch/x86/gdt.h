#pragma once
#include <stdint.h>

typedef struct {
    uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t base_mi;
    uint8_t access;
    uint8_t flags;
    uint8_t base_hi;
} x86_gdt_entry_t;

typedef struct {
    uint16_t size;
    uint32_t offset;
} __attribute__((packed)) x86_gdt_ptr_t;

void x86_gdt_set(int idx, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access);
void x86_gdt_load(void);
