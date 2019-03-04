#include "regs.h"
#include "sys/debug.h"

#define EFLAGS_CF       0x0001
#define EFLAGS_PF       0x0004
#define EFLAGS_AF       0x0010
#define EFLAGS_ZF       0x0040
#define EFLAGS_SF       0x0080
#define EFLAGS_TF       0x0100
#define EFLAGS_IF       0x0200
#define EFLAGS_DF       0x0400
#define EFLAGS_OF       0x0800
#define EFLAGS_NT       0x4000

void x86_dump_gp_regs(int level, const x86_gp_regs_t *regs) {
    kprint(level, "General purpose registers:\n");

    kprint(level, " eax = %d (%p)\n", regs->eax, regs->eax);
    kprint(level, " ecx = %d (%p)\n", regs->ecx, regs->ecx);
    kprint(level, " edx = %d (%p)\n", regs->edx, regs->edx);
    kprint(level, " ebx = %d (%p)\n", regs->ebx, regs->ebx);

    kprint(level, " esp = %d (%p)\n", regs->oesp, regs->oesp);
    kprint(level, " ebp = %d (%p)\n", regs->ebp, regs->ebp);
    kprint(level, " esi = %d (%p)\n", regs->esi, regs->esi);
    kprint(level, " edi = %d (%p)\n", regs->edi, regs->edi);
}

void x86_dump_iret_regs(int level, const x86_iret_regs_t *regs) {
    kprint(level, "Execution flow state:\n");

    kprint(level, " cs:eip = %p:%p\n", regs->cs, regs->eip);
    //if (regs->cs == 0x1B) {
        kprint(level, " ss:esp = %p:%p\n", regs->ss, regs->esp);
    //}

    kprint(level, " eflags = %p\n", regs->eflags);
    kprint(level, "\t(%c%c%c%c%c%c%c%c%c%c IOPL=%d)\n",
            (regs->eflags & EFLAGS_CF) ? 'C' : '-',
            (regs->eflags & EFLAGS_PF) ? 'P' : '-',
            (regs->eflags & EFLAGS_AF) ? 'A' : '-',
            (regs->eflags & EFLAGS_ZF) ? 'Z' : '-',
            (regs->eflags & EFLAGS_SF) ? 'S' : '-',
            (regs->eflags & EFLAGS_TF) ? 'T' : '-',
            (regs->eflags & EFLAGS_IF) ? 'I' : '-',
            (regs->eflags & EFLAGS_DF) ? 'D' : '-',
            (regs->eflags & EFLAGS_OF) ? 'O' : '-',
            (regs->eflags & EFLAGS_NT) ? 'N' : '-',
            (regs->eflags >> 12) & 0x3);
}
