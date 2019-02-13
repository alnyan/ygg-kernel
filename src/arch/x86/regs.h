#pragma once
#include <stdint.h>

typedef struct {
    uint32_t edi, esi, ebp, oesp;
    uint32_t ebx, edx, ecx, eax;
} x86_gp_regs_t;

typedef struct {
    uint32_t eip, cs, eflags, esp, ss;
} x86_iret_regs_t;

typedef struct {
    uint32_t ds, es, fs, gs;
} x86_seg_regs_t;

void x86_dump_gp_regs(const x86_gp_regs_t *regs);
void x86_dump_iret_regs(const x86_iret_regs_t *regs);
