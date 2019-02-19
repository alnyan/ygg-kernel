#include "hw/irq.h"
#include "task/task.h"
#include "arch/hw.h"
#include "mm.h"
#include "hw/console.h"
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
    case SYSCALL_NR_FORK:
        regs->gp.eax = sys_fork();
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
    case SYSCALL_NR_OPEN:
        regs->gp.eax = sys_open((const char *) regs->gp.ebx, (int) regs->gp.ecx, regs->gp.edx);
        break;
    case SYSCALL_NR_CLOSE:
        sys_close((int) regs->gp.ebx);
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

SYSCALL_DEFINE0(fork) {
    task_t *res = task_fork(x86_task_current);

    return res ? ((struct x86_task *) res)->ctl->pid : -1;
}

SYSCALL_DEFINE3(open, const char *path, int flags, uint32_t mode) {
    struct x86_task *t = x86_task_current;
    int free_fd = -1;
    for (int i = 0; i < 4; ++i) {
        if (!t->ctl->fds[i]) {
            free_fd = i;
            break;
        }
    }

    if (free_fd == -1) {
        return -1;
    }

    uint32_t vfsm = 0;
    if (mode & (1 << 0)) {
        vfsm |= VFS_FLG_RD;
    }
    if (mode & (1 << 1)) {
        vfsm |= VFS_FLG_WR;
    }
    vfs_file_t *f = vfs_open(path, vfsm);

    if (!f) {
        return -1;
    }

    t->ctl->fds[free_fd] = f;

    return free_fd;
}

SYSCALL_DEFINE1(close, int fd) {
    struct x86_task *t = x86_task_current;
    if (fd < 0 || fd > 3) {
        return -1;
    }

    vfs_file_t *f;

    if ((f = t->ctl->fds[fd])) {
        vfs_close(f);
        t->ctl->fds[fd] = NULL;
    }

    return 0;
}
