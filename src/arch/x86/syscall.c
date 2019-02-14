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

    switch (regs->gp.eax) {
    case SYSCALL_NR_WRITE:
        regs->gp.eax = sys_write(regs->gp.ebx, (const char *) regs->gp.ecx, regs->gp.edx);
        break;
    default:
        regs->gp.eax = -1;
        break;
    }
}

int sys_write(unsigned int fd, const char *buf, size_t len) {
    // TODO: validate sizes, check if memory is allocated etc.
    for (size_t i = 0; i < len; ++i) {
        x86_con_putc(buf[i]);
    }

    return 0;
}
