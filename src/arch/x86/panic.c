#include "sys/panic.h"
#include "sys/debug.h"
#include "mm.h"
#include "arch/hw.h"

#define X86_PF_FLG_PR   (1 << 0)
#define X86_PF_FLG_RW   (1 << 1)
#define X86_PF_FLG_US   (1 << 2)
#define X86_PF_FLG_ID   (1 << 4)

// Implements IRQ and ISR-specific panics, which dump registers
void panicf_isr(const char *fmt, const x86_int_regs_t *regs, ...) {
    debug(PANIC_MSG_INTRO);
    va_list args;
    va_start(args, regs);
    debugfv(fmt, args);
    va_end(args);

    debug("Exception code: #%u\n", regs->int_no);
    debug("Error code: %d (0x%x)\n", regs->err_code);

    if (regs->int_no == 14) {
        uint32_t cr2;
        asm volatile ("movl %%cr2, %0":"=a"(cr2));

        debug("--- Page fault ---\n");

        debug("Flags: %c%c%c%c\n",
                (regs->err_code & X86_PF_FLG_PR) ? 'P' : '-',
                (regs->err_code & X86_PF_FLG_RW) ? 'W' : 'R',
                (regs->err_code & X86_PF_FLG_US) ? 'U' : '-',
                (regs->err_code & X86_PF_FLG_ID) ? 'I' : 'D');
        debug("CR2 = %p\n", cr2);

        uint32_t cr3;
        asm volatile ("movl %%cr3, %0":"=a"(cr3));

        debug("CR3 = %p\n", cr3);
        if (cr3 == (uintptr_t) mm_kernel - KERNEL_VIRT_BASE) {
            debug("\t(Is kernel)\n");

            uint32_t pde = mm_kernel[cr2 >> 22];
            if (pde & 1) {
                debug("\tEntry for CR2 is present in PD\n");
            } else {
                debug("\tEntry is not mapped in kernel PD\n");
            }
        }
    }

    debug("--- Register dump ---\n");
    x86_dump_gp_regs(&regs->gp);
    x86_dump_iret_regs(&regs->iret);

    panic_hlt();
}

void panicf_irq(const char *fmt, const x86_irq_regs_t *regs, ...) {
    debug(PANIC_MSG_INTRO);
    va_list args;
    va_start(args, regs);
    debugfv(fmt, args);
    va_end(args);

    debug("--- Register dump ---\n");
    x86_dump_gp_regs(&regs->gp);
    x86_dump_iret_regs(&regs->iret);

    panic_hlt();
}
