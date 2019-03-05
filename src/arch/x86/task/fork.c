#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/assert.h"
#include "sys/heap.h"
#include "sys/mem.h"
#include "sys/elf.h"
#include "sys/mm.h"
#include "arch/hw.h"
#include <uapi/errno.h>
#include "sys/task.h"
#include "task.h"

task_t *task_fork(task_t *t) {
    uint32_t cr3_0;
    asm volatile ("mov %%cr3, %0":"=a"(cr3_0));
    assert(cr3_0 == (uint32_t) mm_kernel - KERNEL_VIRT_BASE);

    struct x86_task *src = (struct x86_task *) t;
    assert(src->pd);
    mm_space_t src_pd = src->pd;

    uintptr_t dst_pd_phys;
    mm_space_t dst_pd = mm_create_space(&dst_pd_phys);
    assert(dst_pd);
    assert(mm_space_fork(dst_pd, src_pd, MM_FLG_CLONE_KERNEL | MM_FLG_CLONE_USER) == 0);

    // Here comes new task
    struct x86_task *dst = (struct x86_task *) task_create();
    assert(dst);

    dst->pd = dst_pd;

    struct x86_task_context *src_ctx = (struct x86_task_context *) src->esp0;
    assert(src_ctx);

    struct x86_task_context *dst_ctx = (struct x86_task_context *) heap_alloc(19 * 4);
    assert(dst_ctx);

    assert(dst->ctl);

    dst->esp0 = (uintptr_t) dst_ctx;
    dst->ebp0 = dst->esp0 + 19 * 4;
    dst->esp3_bottom = 0x80000000;
    dst->esp3_size = 4;

    vfs_file_t *fd_tty_wr = vfs_open("/dev/tty0", VFS_FLG_WR);
    assert(fd_tty_wr);
    fd_tty_wr->task = dst;
    dst->ctl->fds[0] = fd_tty_wr;

    vfs_file_t *fd_tty_rd = vfs_open("/dev/tty0", VFS_FLG_RD);
    assert(fd_tty_rd);
    fd_tty_rd->task = dst;
    dst->ctl->fds[1] = fd_tty_rd;

    memcpy(dst_ctx, src_ctx, 19 * 4);

    dst->flag = 0;
    dst_ctx->gp.eax = 0;
    dst_ctx->cr3 = dst_pd_phys;

    task_enable(dst);

    return dst;
}

int task_execve(task_t *dst, const char *path, const char **argp, const char **envp) {
    assert(!argp && !envp);     // These are not supported yet
    struct x86_task *task = (struct x86_task *) dst;

    uint32_t cr3_0;
    asm volatile ("mov %%cr3, %0":"=a"(cr3_0));
    assert(cr3_0 == (uint32_t) mm_kernel - KERNEL_VIRT_BASE);

    struct x86_task_context *ctx = (struct x86_task_context *) task->esp0;
    assert(ctx);

    mm_space_t pd = task->pd;
    assert(pd);

    // TODO: allow loading from sources other than ramdisk
    uintptr_t file_mem = vfs_getm(path);
    if (file_mem == MM_NADDR) {
        return -ENOENT;
    }
    // assert(file_mem != MM_NADDR);

    uintptr_t entry = elf_load(pd, file_mem, 0);
    assert(entry != MM_NADDR);

    memset(&ctx->gp, 0, sizeof(x86_gp_regs_t));

    assert(x86_task_set_context(task, entry, NULL, 0) == 0);

    return 0;
}

task_t *task_fexecve(const char *path, const char **argp, const char **envp) {
    uint32_t cr3_0;
    asm volatile ("mov %%cr3, %0":"=a"(cr3_0));
    assert(cr3_0 == (uint32_t) mm_kernel - KERNEL_VIRT_BASE);

    // TODO: allow loading from sources other than ramdisk
    uintptr_t file_mem = vfs_getm(path);
    assert(file_mem != MM_NADDR);

    // Load the ELF
    mm_pagedir_t pd = mm_create_space(NULL);
    assert(pd);

    mm_space_clone(pd, mm_kernel, MM_FLG_CLONE_KERNEL);

    uintptr_t entry = elf_load(pd, file_mem, 0);
    assert(entry != MM_NADDR);

    // Create and setup the task
    struct x86_task *task = (struct x86_task *) task_create();
    assert(task);

    task->pd = pd;
    task->esp3_bottom = 0;
    task->esp3_size = 4;

    vfs_file_t *fd_tty_wr = vfs_open("/dev/tty0", VFS_FLG_WR);
    assert(fd_tty_wr);
    fd_tty_wr->task = task;
    task->ctl->fds[0] = fd_tty_wr;

    vfs_file_t *fd_tty_rd = vfs_open("/dev/tty0", VFS_FLG_RD);
    assert(fd_tty_rd);
    fd_tty_rd->task = task;
    ((struct x86_task *) task)->ctl->fds[1] = fd_tty_rd;

    assert(x86_task_set_context(task, entry, NULL, 0) == 0);

    task_enable(task);

    return task;
}
