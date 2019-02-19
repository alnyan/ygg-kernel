#include "irq.h"
#include "task.h"
#include "arch/hw.h"
#include "mm.h"
#include "arch/x86/console.h"
#include "sys/vfs.h"
#include "sys/task.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "sys/panic.h"
#include "syscall.h"

// The only code for syscall now: put current task to sleep for some time
void x86_syscall(x86_irq_regs_t *regs) {
    struct x86_task *task = x86_task_current;

    switch (regs->gp.eax) {
    case SYSCALL_NR_EXIT:
        {
            sys_exit(regs->gp.ebx);
            x86_task_switch(regs);
        }
        break;
    case SYSCALL_NR_WRITE:
        {
            int fd = (int) regs->gp.ebx;
            if (fd < 0 || fd > 3) {
                regs->gp.eax = -1;
                break;
            }

            vfs_file_t *file = task->ctl->fds[fd];
            assert(file);

            // TODO: async write
            regs->gp.eax = vfs_write(file, (const void *) regs->gp.ecx, regs->gp.edx);
        }
        break;
    case SYSCALL_NR_READ:
        {
            int fd = (int) regs->gp.ebx;
            if (fd < 0 || fd > 3) {
                regs->gp.eax = -1;
                break;
            }

            vfs_file_t *file = task->ctl->fds[fd];
            assert(file);

            regs->gp.eax = 0;
            int res = vfs_read(file, (void *) regs->gp.ecx, regs->gp.edx, (ssize_t *) &regs->gp.eax);

            if (res == VFS_READ_ASYNC) {
                task_busy(task);

                x86_task_switch(regs);

                break;
            }

            regs->gp.eax = res;
        }
        break;
    default:
        regs->gp.eax = -1;
        break;
    }
}

SYSCALL_DEFINE1(exit, int res) {
    // Exit code is stored in %ebx on task's stack
    x86_task_current->flag |= TASK_FLG_STOP;
    return 0;
}
