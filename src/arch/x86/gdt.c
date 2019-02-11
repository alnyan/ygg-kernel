#include "gdt.h"

// NULL
// CODE K
// DATA K
#define GDT_NENTR   3

static x86_gdt_entry_t s_gdt[GDT_NENTR];
static x86_gdt_ptr_t s_gdtr;

void x86_gdt_load(void) {
    s_gdtr.offset = (uint32_t) s_gdt;
    s_gdtr.size = sizeof(s_gdt) - 1;

    asm volatile ("lgdt (s_gdtr)":::"memory");
}

void x86_gdt_set(int idx, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access) {
    s_gdt[idx].base_lo = base & 0xFFFF;
    s_gdt[idx].base_mi = (base >> 16) & 0xFF;
    s_gdt[idx].base_hi = (base >> 24) & 0xFF;
    s_gdt[idx].access = access;
    s_gdt[idx].flags = (flags & 0xF0) | ((limit >> 16) & 0xF);
    s_gdt[idx].limit_lo = limit & 0xFFFF;
}
