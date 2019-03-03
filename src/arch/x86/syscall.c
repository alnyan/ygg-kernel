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
    mm_set(mm_kernel);

    int res;

    switch (regs->gp.eax) {
    case SYSCALL_NR_EXIT:
        regs->gp.ebx &= 0xFF;
        sys_exit(regs->gp.ebx);
        x86_task_switch(regs);
        break;
    case SYSCALL_NR_FORK:
        regs->gp.eax = sys_fork();
        break;

    case SYSCALL_NR_OPEN:
        regs->gp.eax = sys_open((const userspace char *) regs->gp.ebx, (int) regs->gp.ecx, regs->gp.edx);
        break;
    case SYSCALL_NR_CLOSE:
        sys_close((int) regs->gp.ebx);
        break;
    case SYSCALL_NR_READ:
        if ((res = sys_read(
                        (int) regs->gp.ebx,
                        (userspace void *) regs->gp.ecx,
                        (size_t) regs->gp.edx,
                        (ssize_t *) &regs->gp.eax)) == VFS_READ_ASYNC) {
            task_busy(x86_task_current);
            x86_task_switch(regs);
        } else {
            regs->gp.eax = (uintptr_t) res;
        }
        break;
    case SYSCALL_NR_WRITE:
        regs->gp.eax = (uint32_t) sys_write((int) regs->gp.ebx, (const userspace void *) regs->gp.ecx, (size_t) regs->gp.edx);
        break;

    case SYSCALL_NRX_FEXECVE:
        regs->gp.eax = sys_fexecve((const userspace char *) regs->gp.ebx,
                                   (const userspace char **) regs->gp.ecx,
                                   (const userspace char **) regs->gp.edx);
        break;

    case SYSCALL_NR_GETPID:
        assert(x86_task_current->ctl);
        regs->gp.eax = x86_task_current->ctl->pid;
        break;

    case SYSCALL_NR_NANOSLEEP:
        regs->gp.eax = sys_nanosleep((const userspace struct timespec *) regs->gp.ebx);
        x86_task_switch(regs);
        break;

    default:
        panic_irq("Invalid syscall\n", regs);
    }

    // case SYSCALL_NR_EXECVE:
    //     regs->gp.eax = sys_execve((const userspace char *) regs->gp.ebx,
    //                               (const userspace char **) regs->gp.ecx,
    //                               (const userspace char **) regs->gp.edx);
    //     break;
}

SYSCALL_DEFINE1(exit, int res) {
    // Exit code is stored in %ebx on task's stack
    x86_task_current->flag |= TASK_FLG_STOP;
    return 0;
}

SYSCALL_DEFINE4(read, int fd, userspace void *buf, size_t len, ssize_t *ret) {
    if (fd < 0 || fd >= 4) {
        return -1;
    }

    vfs_file_t *fp;

    if (!(fp = x86_task_current->ctl->fds[fd])) {
        return -1;
    }

    if (fp->flags & (1 << 21)) {
        assert(len == sizeof(struct vfs_dirent));
        struct vfs_dirent dirent_tmp;
        int r = vfs_readdir(fp, &dirent_tmp);
        if (r >= 0) {
            mm_memcpy_kernel_to_user(task_space(x86_task_current), buf, &dirent_tmp, sizeof(struct vfs_dirent));
        }
        return r;
    } else {
        assert(ret);
        *ret = 0;
        return vfs_read(fp, buf, len, (ssize_t *) ret);
    }
}

SYSCALL_DEFINE3(write, int fd, const userspace void *buf, size_t len) {
    // TODO: async write
    if (fd < 0 || fd >= 4) {
        return -1;
    }

    vfs_file_t *fp;

    if (!(fp = x86_task_current->ctl->fds[fd])) {
        return -1;
    }

    static char tmp_buf[512];
    size_t bytes_left = len;
    ssize_t bytes_written = 0;

    // TODO: vfs_write_user
    while (bytes_left) {
        size_t bytes_copy = bytes_left;
        if (bytes_copy > sizeof(tmp_buf)) {
            bytes_copy = sizeof(tmp_buf);
        }
        assert(mm_memcpy_user_to_kernel(x86_task_current->pd, tmp_buf, buf, bytes_copy) == 0);

        ssize_t res = vfs_write(fp, tmp_buf, bytes_copy);

        if (res != bytes_copy) {
            if (bytes_written == 0) {
                bytes_written = res;
            }

            break;
        } else {
            bytes_written += res;
            bytes_left -= bytes_copy;
        }
    }

    return bytes_written;
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

    char path_cloned[256];
    assert(mm_strncpy_user_to_kernel(task_space(x86_task_current), path_cloned, path, sizeof(path_cloned)) > 0);

    uint32_t vfsm = 0;
    // O_RDONLY
    if (mode & (1 << 0)) {
        vfsm |= VFS_FLG_RD;
    }
    // O_WRONLY
    if (mode & (1 << 1)) {
        vfsm |= VFS_FLG_WR;
    }
    // O_DIRECTORY
    if (flags & (1 << 21)) {
        vfs_file_t *f = vfs_opendir(path_cloned);

        if (!f) {
            return -1;
        }

        t->ctl->fds[free_fd] = f;

        return free_fd;
    } else {
        vfs_file_t *f = vfs_open(path_cloned, vfsm);

        if (!f) {
            return -1;
        }

        t->ctl->fds[free_fd] = f;

        return free_fd;
    }
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
    mm_strncpy_user_to_kernel(task_space(x86_task_current), path_tmp, path, sizeof(path_tmp));
    task_t *res = task_fexecve(path_tmp, argp, envp);
    return res ? 0 : -1;
}

SYSCALL_DEFINE1(nanosleep, const userspace struct timespec *ts_user) {
    struct timespec ts;
    mm_memcpy_user_to_kernel(task_space(x86_task_current), &ts, ts_user, sizeof(struct timespec));

    if (ts.tv_sec || ts.tv_nsec) {
        task_set_sleep(x86_task_current, &ts);
        x86_task_current->flag |= TASK_FLG_WAIT;
    }

    return 0;
}

// SYSCALL_DEFINE3(execve, const userspace char *path, const userspace char **argp, const userspace char **envp) {
//     // Not supported yet
//     assert(!argp || !envp);
//     char path_tmp[256];
//     task_copy_from_user(x86_task_current, path_tmp, path, MM_NADDR);
//     return task_execve(x86_task_current, path_tmp, argp, envp);
// }
