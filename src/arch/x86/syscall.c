#include "irq.h"
#include "task.h"
#include "arch/hw.h"
#include "mm.h"
#include "sys/task.h"
#include "sys/debug.h"
#include "syscall.h"

static volatile int cntr = 0;

// The only code for syscall now: put current task to sleep for some time
void x86_syscall(x86_irq_regs_t *regs) {
    struct x86_task *t = x86_task_current;

    switch (regs->gp.eax) {
    default:
        regs->gp.eax = ++cntr;
        break;
    }
}

int sys_write(unsigned int fd, const char *buf, size_t len) {
    debug("sys_write: %d, %p, %u\n", fd, buf, len);
}
