#include "irq.h"
#include "sys/panic.h"
#include "sys/debug.h"
#include "sys/task.h"
#include "sys/assert.h"

static void x86_page_fault_dump(int crash, int level, x86_irq_regs_t *regs) {
    uintptr_t cr2;
    asm volatile ("mov %%cr2, %0":"=a"(cr2));

    kprint(level, "---- Page fault ----\n");

    kprint(level, " Address: %p\n", cr2);

    if (regs->iret.cs == 0x08) {
        kprint(level, "Kernel error\n");
    } else {
        kprint(level, "User protection violation\n");

        if (x86_task_current) {
            kprint(level, "---- Context dump ----\n");
            x86_task_dump_context(level, x86_task_current);
        }
    }

    kprint(level, "---- Register dump ----\n");
    x86_dump_gp_regs(level, &regs->gp);
    x86_dump_iret_regs(level, &regs->iret);

    if (crash) {
        panic_hlt();
    }
}

void x86_page_fault(x86_irq_regs_t *regs) {
    if (regs->iret.cs == 0x08) {
        x86_page_fault_dump(1, DEBUG_FATAL, regs);
    } else {
        // Debug dump
        x86_page_fault_dump(0, DEBUG_DEFAULT, regs);

        // Disable current task
        x86_task_current->flag |= TASK_FLG_STOP;
        // This will indicate an error
        regs->gp.ecx = TASK_EXIT_SEGV;
        x86_task_switch(regs);
    }
}
