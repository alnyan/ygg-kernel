#include "sys/panic.h"
#include "sys/debug.h"
#include "mm.h"
#include "arch/hw.h"
#include "sys/heap.h"
#include "sys/mm.h"

#define X86_PF_FLG_PR   (1 << 0)
#define X86_PF_FLG_RW   (1 << 1)
#define X86_PF_FLG_US   (1 << 2)
#define X86_PF_FLG_ID   (1 << 4)

void panic_reg(void) {
    struct heap_stat st;
    heap_stat(&st);
    heap_dump();

    kfatal("---- HEAP DUMP ----\n");
    kfatal("Heap free: %u\n", st.free);
}

// Implements IRQ and ISR-specific panics, which dump registers
void panicf_isr(const char *fmt, const x86_int_regs_t *regs, ...) {
    kfatal(PANIC_MSG_INTRO);
    va_list args;
    va_start(args, regs);
    debugfv(DEBUG_FATAL, fmt, args);
    va_end(args);

    kfatal("Exception code: #%u\n", regs->int_no);
    kfatal("Error code: %d (0x%x)\n", regs->err_code);

    kfatal("--- Register dump ---\n");
    x86_dump_gp_regs(DEBUG_FATAL, &regs->gp);
    x86_dump_iret_regs(DEBUG_FATAL, &regs->iret);

    panic_hlt();
}

void panicf_irq(const char *fmt, const x86_irq_regs_t *regs, ...) {
    kfatal(PANIC_MSG_INTRO);
    va_list args;
    va_start(args, regs);
    debugfv(DEBUG_FATAL, fmt, args);
    va_end(args);

    kfatal("--- Register dump ---\n");
    x86_dump_gp_regs(DEBUG_FATAL, &regs->gp);
    x86_dump_iret_regs(DEBUG_FATAL, &regs->iret);

    panic_hlt();
}
