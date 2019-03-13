#include "irq.h"
#include "sys/panic.h"
#include "sys/debug.h"
#include "sys/task.h"
#include "sys/sched.h"
#include "sys/assert.h"
#include "sys/signal.h"

static void x86_page_fault_dump(int crash, int level, x86_irq_regs_t *regs) {
    uintptr_t cr2;
    asm volatile ("mov %%cr2, %0":"=a"(cr2));

    kprint(level, "---- Page fault ----\n");

    kprint(level, " Address: %p\n", cr2);

    kprint(level, "---- Register dump ----\n");
    x86_dump_gp_regs(level, &regs->gp);
    x86_dump_iret_regs(level, &regs->iret);

    if (sched_current) {
        mm_dump_map(level, task_space(sched_current));
    }

    if (crash) {
        panic_hlt();
    }
}

void x86_page_fault(x86_irq_regs_t *regs) {
    if (regs->iret.cs == 0x08) {
        x86_page_fault_dump(1, DEBUG_FATAL, regs);
    } else {
        x86_page_fault_dump(1, DEBUG_FATAL, regs);

        // TODO: handle this condition
    }
}
