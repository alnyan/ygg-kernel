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

void x86_dump_gp_regs(const x86_gp_regs_t *regs) {
    debug("General purpose registers:\n");

    debug(" eax = %d (0x%x)\n", regs->eax, regs->eax);
    debug(" ecx = %d (0x%x)\n", regs->ecx, regs->ecx);
    debug(" edx = %d (0x%x)\n", regs->edx, regs->edx);
    debug(" ebx = %d (0x%x)\n", regs->ebx, regs->ebx);

    debug(" esp = %d (0x%x)\n", regs->oesp, regs->oesp);
    debug(" ebp = %d (0x%x)\n", regs->ebp, regs->ebp);
    debug(" esi = %d (0x%x)\n", regs->esi, regs->esi);
    debug(" edi = %d (0x%x)\n", regs->edi, regs->edi);
}

void x86_dump_iret_regs(const x86_iret_regs_t *regs) {
    debug("Execution flow state:\n");

    debug(" cs:eip = 0x%x:0x%x\n", regs->cs, regs->eip);
    if (regs->cs == 0x1B) {
        debug(" ss:esp = 0x%x:0x%x\n", regs->ss, regs->esp);
    }

    debug(" eflags = 0x%x\n", regs->eflags);
    debug("\t(%c%c%c%c%c%c%c%c%c%c IOPL=%d)\n",
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
