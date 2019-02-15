#include "irq.h"
#include "task.h"
#include "arch/hw.h"
#include "mm.h"
#include "sys/task.h"
#include "sys/debug.h"
#include "dev/vfs.h"
#include "syscall.h"

// The only code for syscall now: put current task to sleep for some time
void x86_syscall(x86_irq_regs_t *regs) {
    struct x86_task *t = x86_task_current;
    int ret;

    switch (regs->gp.eax) {
    case SYSCALL_NR_READ:
        {
            int fd = regs->gp.ebx;
            ssize_t res;

            if (fd >= sizeof(x86_task_current->ctl->files) / sizeof(vfs_file_t *)) {
                regs->gp.eax = -1;
                break;
            }

            char *data = (char *) regs->gp.ecx;
            size_t len = (size_t) regs->gp.edx;

            vfs_file_t *f = x86_task_current->ctl->files[fd];

            if (f->f_dev && (f->f_dev->flags & DEV_FLG_RDAS)) {
                if ((res = vfs_read(f, data, len)) < 0) {
                    // This means immediate error
                    regs->gp.eax = res;
                    break;
                }

                f->res = &regs->gp.eax;
                regs->gp.eax = 0;
                x86_task_switch(regs);
            } else {
                regs->gp.eax = vfs_read(f, data, len);
            }
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
