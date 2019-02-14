#include "irq.h"
#include "task.h"
#include "arch/hw.h"
#include "mm.h"
#include "sys/task.h"
#include "sys/debug.h"

// The only code for syscall now: put current task to sleep for some time
void x86_syscall(x86_irq_regs_t *regs) {
    struct x86_task *t = x86_task_current;

    t->ctl->sleep = regs->gp.eax + 20;
    t->flag |= TASK_FLG_WAIT;

    x86_task_switch(regs);
}
