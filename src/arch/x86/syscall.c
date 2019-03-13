#include "hw/irq.h"
#include "sys/sched.h"
#include "sys/task.h"
#include "sys/debug.h"
#include <uapi/syscall.h>
#include <uapi/signum.h>

int x86_syscall(x86_irq_regs_t *regs) {
    x86_task_t *task = (x86_task_t *) sched_current;

    switch (regs->gp.eax) {
    case SYSCALL_NR_EXIT:
        task_terminate(task, 0);
        sched();
        return 1;

    case SYSCALL_NRX_SIGNAL:
        // TODO
        return 0;

    default:
        kinfo("[%d] invalid syscall: 0x%x\n", task_ctl(task)->pid, regs->gp.eax);
        task_terminate(task, SIGSEGV);
        sched();
        return 1;
    }
}
