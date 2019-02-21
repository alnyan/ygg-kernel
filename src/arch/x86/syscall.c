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
#include "sys/mm.h"
#include "syscall.h"

// The only code for syscall now: put current task to sleep for some time
void x86_syscall(x86_irq_regs_t *regs) {
    if (regs->gp.eax != SYSCALL_NR_WRITE && regs->gp.eax != SYSCALL_NR_READ) {
        mm_set_kernel();
    }
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
        regs->gp.eax = sys_open((const userspace char *) regs->gp.ebx, (int) regs->gp.ecx, regs->gp.edx);
        break;
    case SYSCALL_NR_CLOSE:
        sys_close((int) regs->gp.ebx);
        break;

    case SYSCALL_NR_EXECVE:
        regs->gp.eax = sys_execve((const userspace char *) regs->gp.ebx,
                                  (const userspace char **) regs->gp.ecx,
                                  (const userspace char **) regs->gp.edx);
        break;
    // Non-standard fork() + execve() syscall
    case SYSCALL_NRX_FEXECVE:
        regs->gp.eax = sys_fexecve((const userspace char *) regs->gp.ebx,
                                   (const userspace char **) regs->gp.ecx,
                                   (const userspace char **) regs->gp.edx);
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

SYSCALL_DEFINE3(open, const userspace char *path, int flags, uint32_t mode) {
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

SYSCALL_DEFINE3(fexecve, const userspace char *path, const userspace char **argp, const userspace char **envp) {
    // Not supported yet
    assert(!argp || !envp);
    char path_tmp[256];
    task_copy_from_user(x86_task_current, path_tmp, path, MM_NADDR);
    task_t *res = task_fexecve(path_tmp, argp, envp);
    return res ? 0 : -1;
}

SYSCALL_DEFINE3(execve, const userspace char *path, const userspace char **argp, const userspace char **envp) {
    // Not supported yet
    assert(!argp || !envp);
    char path_tmp[256];
    task_copy_from_user(x86_task_current, path_tmp, path, MM_NADDR);
    return task_execve(x86_task_current, path_tmp, argp, envp);
}
