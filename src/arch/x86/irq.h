#pragma once
#include "regs.h"
#include "io.h"

#define irq_enable() asm volatile("sti")
#define irq_disable() asm volatile("cli")

static inline void x86_irq_eoi(int n) {
    if (n >= 8) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

typedef struct {
    x86_seg_regs_t segs;
    x86_gp_regs_t gp;
    x86_iret_regs_t iret;
} x86_irq_regs_t;

void pic8259_init(void);
