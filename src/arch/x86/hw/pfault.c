#include "irq.h"
#include "sys/panic.h"
#include "sys/debug.h"
#include "sys/task.h"
#include "sys/assert.h"
#include "sys/signal.h"

static void x86_page_fault_dump(int crash, int level, x86_irq_regs_t *regs) {
    uintptr_t cr2;
    asm volatile ("mov %%cr2, %0":"=a"(cr2));

    kprint(level, "---- Page fault ----\n");

    kprint(level, " Address: %p\n", cr2);

#if defined(ENABLE_TASK)
    if (regs->iret.cs == 0x08) {
        kprint(level, "Kernel error\n");
    } else {
        kprint(level, "User protection violation\n");

        if (x86_task_current) {
            kprint(level, "---- Context dump ----\n");
            x86_task_dump_context(level, x86_task_current);
        }
    }
#endif

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
#if defined(ENABLE_TASK)
        // uint32_t *sfix = (uint32_t *) regs;
        // uint32_t error_code = sfix[18];

        // for (int i = 13; i < 18; ++i) {
        //     sfix[i] = sfix[i + 1];
        // }

        // kdebug("ERROR CODE 0x%x\n", error_code);

        if (x86_task_current->ctl->pending_signal) {
            // Disable current task
            x86_task_current->flag |= TASK_FLG_STOP;
            // This will indicate an error
            regs->gp.ebx = TASK_EXIT_SEGV;

            x86_task_switch(regs);
        } else {
            // TODO: task_signal()
            x86_task_current->ctl->pending_signal = SIGSEGV;
            if (x86_task_enter_signal(x86_task_current) != 0) {
                kwarn("Task %d has no signal entry point\n", x86_task_current->ctl->pid);
                // Disable current task
                x86_task_current->flag |= TASK_FLG_STOP;
                // This will indicate an error
                regs->gp.ebx = TASK_EXIT_SEGV;

                x86_task_switch(regs);
            }
        }
#else
        while (1);
#endif
    }
}
