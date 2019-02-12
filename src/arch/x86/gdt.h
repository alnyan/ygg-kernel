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
    uint32_t link;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;  // UNUSED
    uint32_t ss1;   // UNUSED
    uint32_t esp2;  // UNUSED
    uint32_t ss2;   // UNUSED
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t gp_regs[8];
    uint32_t seg_regs[6];
    uint32_t ldtr;
    uint32_t flags;
} x86_tss_entry_t;

typedef struct {
    uint16_t size;
    uint32_t offset;
} __attribute__((packed)) x86_gdt_ptr_t;

void x86_gdt_set(int idx, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access);
void x86_tss_set(uint32_t esp0);
void x86_gdt_load(void);
void gdt_init(void);
