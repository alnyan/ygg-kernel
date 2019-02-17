#include "irq.h"
#include "task.h"
#include "arch/hw.h"
#include "mm.h"
#include "arch/x86/console.h"
#include "sys/task.h"
#include "sys/debug.h"
#include "syscall.h"

// The only code for syscall now: put current task to sleep for some time
void x86_syscall(x86_irq_regs_t *regs) {
    switch (regs->gp.eax) {
    case SYSCALL_NR_EXIT:
        {
            sys_exit(regs->gp.ebx);
            x86_task_switch(regs);
        }
        break;
    case SYSCALL_NR_READ:
        regs->gp.eax = (uint32_t) -1;
        break;
    case SYSCALL_NR_WRITE:
        regs->gp.eax = sys_write(regs->gp.ebx, (const void *) regs->gp.ecx, regs->gp.edx);
        break;
    default:
        regs->gp.eax = -1;
        break;
    }
}

SYSCALL_DEFINE3(write, int fd, const void *data, size_t len) {
    for (int i = 0; i < len; ++i) {
        x86_con_putc(((const char *) data)[i]);
    }
    return len;
}

SYSCALL_DEFINE1(exit, int res) {
    // Exit code is stored in %ebx on task's stack
    x86_task_current->flag |= TASK_FLG_STOP;
    return 0;
}
