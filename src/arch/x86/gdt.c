#include "gdt.h"
#include "sys/debug.h"
#include "sys/mem.h"

// NULL
// CODE K
// DATA K
// CODE U
// DATA U
#define GDT_NENTR   6

#define GDT_ACC_AC  (1 << 0)
#define GDT_ACC_RW  (1 << 1)
#define GDT_ACC_DC  (1 << 2)
#define GDT_ACC_EX  (1 << 3)
#define GDT_ACC_S   (1 << 4)
#define GDT_ACC_R0  (0 << 5)
#define GDT_ACC_R1  (1 << 5)
#define GDT_ACC_R2  (2 << 5)
#define GDT_ACC_R3  (3 << 5)
#define GDT_ACC_PR  (1 << 7)

#define GDT_FLG_SZ  (1 << 6)
#define GDT_FLG_GR  (1 << 7)

x86_tss_entry_t x86_tss;
static x86_gdt_entry_t s_gdt[GDT_NENTR];
static x86_gdt_ptr_t s_gdtr;

void x86_tss_set(uint32_t esp0) {
    debug("%p -> %p\n", x86_tss.esp0, esp0);
    x86_tss.esp0 = esp0;
}

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

void gdt_init(void) {
    debug("Setting up GDT entries\n");

    memset(&x86_tss, 0, sizeof(x86_tss));
    x86_tss.ss0 = 0x10;
    x86_tss.flags = sizeof(x86_tss) << 16;

    x86_gdt_set(0, 0, 0, 0, 0);
    x86_gdt_set(1, 0, 0xFFFFF,
            GDT_FLG_GR | GDT_FLG_SZ,
            GDT_ACC_PR | GDT_ACC_R0 | GDT_ACC_EX | GDT_ACC_S);
    x86_gdt_set(2, 0, 0xFFFFF,
            GDT_FLG_GR | GDT_FLG_SZ,
            GDT_ACC_PR | GDT_ACC_R0 | GDT_ACC_RW | GDT_ACC_S);
    x86_gdt_set(3, 0, 0xFFFFF,
            GDT_FLG_GR | GDT_FLG_SZ,
            GDT_ACC_PR | GDT_ACC_R3 | GDT_ACC_EX | GDT_ACC_S);
    x86_gdt_set(4, 0, 0xFFFFF,
            GDT_FLG_GR | GDT_FLG_SZ,
            GDT_ACC_PR | GDT_ACC_R3 | GDT_ACC_RW | GDT_ACC_S);
    x86_gdt_set(5, (uint32_t) &x86_tss, sizeof(x86_tss),
            GDT_FLG_SZ,
            GDT_ACC_PR | GDT_ACC_EX | GDT_ACC_AC);

    x86_gdt_load();

    asm volatile ("movw $0x28, %%ax; ltr %%ax":::"memory");
}
