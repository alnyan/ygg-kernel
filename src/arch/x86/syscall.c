#include "hw/irq.h"
#include "sys/sched.h"
#include "sys/task.h"
#include "sys/debug.h"
#include "fs/vfs.h"
#include "syscall.h"
#include <uapi/errno.h>
#include <uapi/syscall.h>
#include <uapi/signum.h>

int x86_syscall(x86_irq_regs_t *regs) {
    x86_task_t *task = (x86_task_t *) sched_current;

    switch (regs->gp.eax) {
    case SYSCALL_NR_EXIT:
        task_terminate(task, 0);
        sched();
        return 1;

    case SYSCALL_NR_READ:
        regs->gp.eax = sys_read(task, (int) regs->gp.ebx, (userspace void *) regs->gp.ecx, regs->gp.edx);
        return 0;
    case SYSCALL_NR_WRITE:
        regs->gp.eax = sys_write(task, (int) regs->gp.ebx, (const userspace void *) regs->gp.ecx, regs->gp.edx);
        return 0;

    case SYSCALL_NRX_FEXECVE:
        regs->gp.eax = sys_fexecve(task, (const userspace char *) regs->gp.ebx,
                                         (const userspace char **) regs->gp.ecx,
                                         (const userspace char **) regs->gp.edx);
        return 0;

    case SYSCALL_NR_WAITPID:
        regs->gp.eax = sys_waitpid(task, (pid_t) regs->gp.ebx, (userspace int *) regs->gp.ecx, (int) regs->gp.edx);
        return 0;

    case SYSCALL_NR_GETPID:
        regs->gp.eax = task_ctl(task)->pid;
        return 0;

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

#define SYSCALL_RWBUF_SIZE  512

SYSCALL_DEFINE3(read, int fd, userspace void *buf, size_t count) {
    vfs_node_t *node = task_get_fd(task, fd);
    if (!node) {
        return -EBADF;
    }

    ssize_t w = 0;

    // Just split large reads to small ones
    while (count) {
        size_t l = count;
        if (l > SYSCALL_RWBUF_SIZE) {
            l = SYSCALL_RWBUF_SIZE;
        }

        ssize_t cw = vfs_read(node, (userspace void *) ((uintptr_t) buf + w), l);

        if (cw != l) {
            if (cw < 0) {
                w = cw;
            } else {
                w += cw;
            }

            break;
        }

        w += l;
        count -= l;
    }


    return w;
}

SYSCALL_DEFINE3(write, int fd, const userspace void *buf, size_t count) {
    vfs_node_t *node = task_get_fd(task, fd);
    if (!node) {
        return -EBADF;
    }

    char kbuf[SYSCALL_RWBUF_SIZE];
    ssize_t w = 0;

    while (count) {
        size_t l = count;
        if (l > sizeof(kbuf)) {
            l = sizeof(kbuf);
        }

        if (mm_memcpy_user_to_kernel(task_space(task), kbuf, (const void *) ((uintptr_t) buf + w), l) != 0) {
            w = -1;
            break;
        }

        ssize_t cw = vfs_write(node, kbuf, l);

        if (cw != l) {
            if (cw < 0) {
                w = cw;
            } else {
                w += cw;
            }

            break;
        }

        w += l;
        count -= l;
    }

    return w;
}

SYSCALL_DEFINE3(fexecve, const userspace char *path, const userspace char **argp, const userspace char **envp) {
    if (argp || envp) {
        // Not supported
        return -EINVAL;
    }

    char namebuf[256];
    namebuf[255] = 0;
    mm_strncpy_user_to_kernel(task_space(task), namebuf, path, 255);

    return task_fexecve(namebuf, NULL, NULL);
}

SYSCALL_DEFINE3(waitpid, pid_t pid, userspace int *wstatus, int options) {
    kdebug("Waitpid %d\n", pid);

    task_ctl(task)->wait.wait_pid = sched_find(pid);
    if (!task_ctl(task)->wait.wait_pid) {
        return -EINVAL;
    }
    task_busy(task, TASK_BUSY_PID);

    asm volatile ("sti");
    while (task_ctl(task)->flags & TASK_FLG_BUSY) {
        asm volatile ("hlt");
    }
    asm volatile ("cli");
    kdebug("Resume after waitpid\n");

    return 0;
}
