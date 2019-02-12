#pragma once

typedef struct {
    uint32_t edi, esi, ebp, oesp;
    uint32_t ebx, edx, ecx, eax;
} x86_gp_regs_t;

typedef struct {
    uint32_t eip, cs, eflags, esp, ss;
} x86_iret_regs_t;
