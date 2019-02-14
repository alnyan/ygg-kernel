#include "sys/panic.h"
#include "sys/debug.h"

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
        debug("TODO: implement dumps for page fault\n");
    }

    debug("--- Register dump ---\n");
    x86_dump_gp_regs(&regs->gp);
    x86_dump_iret_regs(&regs->iret);

    panic_hlt();
}
