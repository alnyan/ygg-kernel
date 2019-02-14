#include "irq.h"
#include "task.h"
#include "arch/hw.h"
#include "mm.h"
#include "sys/task.h"
#include "sys/debug.h"
#include "syscall.h"

// The only code for syscall now: put current task to sleep for some time
void x86_syscall(x86_irq_regs_t *regs) {
    struct x86_task *t = x86_task_current;
    int ret;

    switch (regs->gp.eax) {
    case SYSCALL_NR_READ:
        if ((ret = sys_read(regs->gp.ebx, (void *) regs->gp.ecx, regs->gp.edx)) != 0) {
            regs->gp.eax = ret;
        } else {
            // Task is now busy, switch to next one
            x86_task_switch(regs);
        }
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

SYSCALL_DEFINE3(read, int fd, void *data, size_t len) {
    x86_task_current->ctl->readc = len;
    x86_task_current->flag |= TASK_FLG_BUSY;
    return 0;
}
