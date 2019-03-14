#include "task.h"
#include "sys/task.h"
#include "sys/sched.h"
#include "sys/elf.h"
#include <uapi/errno.h>
#include "sys/assert.h"
#include "arch/hw.h"
#include "dev/tty.h"
#include "sys/debug.h"
#include "fs/vfs.h"

int task_fexecve(const char *path, const char **argp, const char **envp) {
    asm volatile ("cli");

    // Obtain the file
    vfs_node_t *fd = vfs_find_node(sched_current, "/bin/init");
    if (!fd) {
        kdebug("vfs_find_node = ENOENT\n");
        return -ENOENT;
    }

    if (!(fd->flags & VFS_NODE_FLG_MEMR)) {
        // Need to read the whole file into buffer
        panic("NYI\n");
    }
    // Just get the memory where the file data is, as it is in-memory file
    uintptr_t addr = vfs_getm(fd);
    assert(addr != MM_NADDR);

    // Load the ELF
    uintptr_t pd_phys;
    mm_pagedir_t pd = mm_create_space(&pd_phys);
    assert(pd);

    mm_space_clone(pd, mm_kernel, MM_FLG_CLONE_KERNEL);

    uintptr_t entry = elf_load(pd, addr, 0);
    assert(entry != MM_NADDR);

    // Create and setup the task
    task_t *task = task_create();
    assert(task);

    assert(x86_task_space(task, pd, pd_phys) == 0);
    assert(x86_task_eip(task, 0x1B, entry) == 0);
    assert(x86_task_user_alloc(task, 0x80000000, 4) == 0);
    // struct x86_task *task = (struct x86_task *) task_create();
    // assert(task);

    // task->pd = pd;
    // task->esp3_bottom = 0;
    // task->esp3_size = 4;

    // TODO: vfs_open
    vfs_node_t *fd_tty_wr = vfs_node_create();
    assert(fd_tty_wr);
    fd_tty_wr->flags = VFS_NODE_TYPE_CHR;
    fd_tty_wr->fd_dev.dev = dev_tty;
    fd_tty_wr->task = task;
    task_ctl(task)->fds[0] = fd_tty_wr;
    vfs_node_t *fd_tty_rd = vfs_node_create();
    assert(fd_tty_rd);
    fd_tty_rd->flags = VFS_NODE_TYPE_CHR;
    fd_tty_rd->fd_dev.dev = dev_tty;
    fd_tty_rd->task = task;
    task_ctl(task)->fds[1] = fd_tty_rd;

    // vfs_node_t *fd_tty_rd = vfs_node_create();
    // assert(fd_tty_rd);
    // fd_tty_rd->flags = VFS_NODE_TYPE_CHR;
    // fd_tty_rd->fd_dev.dev = dev_tty;
    // fd_tty_rd->task = task;
    // task->ctl->fds[1] = fd_tty_rd;
    // vfs_file_t *fd_tty_wr = vfs_open("/dev/tty0", VFS_FLG_WR);
    // assert(fd_tty_wr);
    // fd_tty_wr->task = task;
    // task->ctl->fds[0] = fd_tty_wr;

    // vfs_file_t *fd_tty_rd = vfs_open("/dev/tty0", VFS_FLG_RD);
    // assert(fd_tty_rd);
    // fd_tty_rd->task = task;
    // ((struct x86_task *) task)->ctl->fds[1] = fd_tty_rd;

    // assert(x86_task_set_context(task, entry, NULL, 0) == 0);

    assert(sched_add(task) > 0);

    asm volatile ("sti");
    return 0;
}
