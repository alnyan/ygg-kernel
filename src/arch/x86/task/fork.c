#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/assert.h"
#include "sys/heap.h"
#include "sys/mem.h"
#include "sys/elf.h"
#include "sys/mm.h"
#include "arch/hw.h"
#include "sys/task.h"
#include "task.h"

extern int x86_last_pid;

static void task_copy_pages(mm_pagedir_t dst, const mm_pagedir_t src) {
    mm_clone(dst, mm_kernel);

    // XXX: The following method is reeeeaaaaalllyyy stupid
    for (uint32_t i = 0; i < (KERNEL_VIRT_BASE - 1) >> 22; ++i) {
        if (src[i] & 1) {
            uint32_t src_phys = src[i] & -0x400000;
            uint32_t dst_phys = mm_alloc_phys_page();

            dst[i] = dst_phys | (src[i] & 0x3FFFFF);

            assert(dst_phys != MM_NADDR);

            if (mm_current != mm_kernel) {
                debug("mm_current == mm_kernel, that's probably not a good idea\n");
            }

            debug("Physically copying %p -> %p\n", src_phys, dst_phys);

            // Map both of them somewhere
            x86_mm_map(mm_current, 0xF0000000, dst_phys, X86_MM_FLG_PS | X86_MM_FLG_RW);
            x86_mm_map(mm_current, 0xF0400000, src_phys, X86_MM_FLG_PS);

            memcpy((void *) 0xF0000000, (const void *) 0xF0400000, 0x400000);

            mm_unmap_cont_region(mm_current, 0xF0000000, 2, 0);
        }
    }
}

task_t *task_fork(task_t *t) {
    struct x86_task *src = (struct x86_task *) t;

    // TODO: smarter cloning, don't just blindly clone all sub-kernel pages
    //  * Probably use COW
    //  * Use some markers to tell, for example, .text-pages from .data ones, as fork()s may share
    //   the same code
    uint32_t src_cr3 = *((uint32_t *) (src->ebp0 - 14 * 4));
    mm_pagedir_t src_pd = (mm_pagedir_t) (src_cr3 + KERNEL_VIRT_BASE);
    mm_pagedir_t dst_pd = mm_pagedir_alloc();
    assert(dst_pd);

    // Data copying happens here
    task_copy_pages(dst_pd, src_pd);

    // Clone registers on stack
    uint32_t dst_ebp0 = (uint32_t) heap_alloc(18 * 4) + 18 * 4;
    memcpy((void *) (dst_ebp0 - 18 * 4), (const void *) (src->ebp0 - 18 * 4), 18 * 4);

    // However, we need to replace cr3
    *((uint32_t *) (dst_ebp0 - 14 * 4)) = (uintptr_t) dst_pd - KERNEL_VIRT_BASE;
    // Fork returns 0
    *((uint32_t *) (dst_ebp0 - 6 * 4)) = 0;

    // Here comes new task
    struct x86_task *dst = (struct x86_task *) task_create();
    assert(dst);

    // Setup stacks
    dst->ebp0 = dst_ebp0;
    dst->ebp3 = src->ebp3;
    dst->esp0 = dst_ebp0 - 18 * 4;

    // Give the task a new PID
    dst->ctl->pid = ++x86_last_pid;
    // TODO: actually clone open descriptors
    dst->ctl->fds[0] = vfs_open("/dev/tty0", VFS_FLG_WR | VFS_FLG_RD);

    // Add it to sched
    task_enable(dst);

    return dst;
}

task_t *task_fexecve(const char *path, const char **argp, const char **envp) {
    // TODO: allow loading from sources other than ramdisk
    uintptr_t file_mem = vfs_getm(path);
    assert(file_mem != MM_NADDR);

    uint32_t kernel_cr3 = (uint32_t) mm_kernel - KERNEL_VIRT_BASE;
    asm volatile ("mov %0, %%cr3"::"a"(kernel_cr3));

    // Load the ELF
    mm_pagedir_t pd = mm_pagedir_alloc();
    mm_clone(pd, mm_kernel);

    uintptr_t entry = elf_load(pd, file_mem, 0);

    // Create and setup the task
    task_t *task = task_create();
    extern int x86_task_setup_stack(struct x86_task *t,
        void (*entry)(void *),
        void *arg,
        mm_pagedir_t pd,
        uint32_t ebp0,
        uint32_t ebp3,
        int flag);
    uint32_t ebp0 = (uint32_t) heap_alloc(18 * 4) + 18 * 4;
    uint32_t ebp3 = 0x80000000 + 0x400000;

    vfs_file_t *fd_tty_rd = vfs_open("/dev/tty0", VFS_FLG_RD | VFS_FLG_WR);
    fd_tty_rd->task = task;
    assert(fd_tty_rd);
    ((struct x86_task *) task)->ctl->fds[0] = fd_tty_rd;
    x86_mm_map(pd, 0x80000000, mm_alloc_phys_page(), X86_MM_FLG_US | X86_MM_FLG_RW | X86_MM_FLG_PS);

    assert(x86_task_setup_stack(
        (struct x86_task *) task,
        (void(*)(void *)) entry,
        NULL,
        pd,
        ebp0,
        ebp3,
        0
    ) == 0);

    task_enable(task);

    return task;
}
