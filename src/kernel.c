#include "sys/assert.h"
#include "dev/initrd.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "dev/devfs.h"
#include "sys/task.h"
#include "dev/tty.h"
#include "arch/hw.h"
#include "sys/elf.h"
#include "sys/mm.h"
#include "util.h"

#include "arch/x86/task.h"
#include "sys/heap.h"

void kernel_main(void) {
    // Init basic stuff so we can at least print something
    hw_early_init();
    // Init printing
    debug_init();
    // Proceed on hw-specific details of init
    hw_init();

    // Now the kernel-stuff kicks in
    devfs_init();
    tty_init();
    // Will create basic device set
    devfs_populate();

    assert(vfs_mount(NULL, "/dev", vfs_devfs, 0) == 0);
    assert(vfs_mount("/dev/ram0", "/", vfs_initramfs, 0) == 0);

    vfs_dirent_t ent;
    vfs_dir_t *dir;
    assert(dir = vfs_opendir("/dev"));

    while (vfs_readdir(dir, &ent) == 0) {
        vfs_dirent_dump(&ent);
    }

    vfs_closedir(dir);

    ////

    task_t *task = task_create();
    uintptr_t init_file = vfs_getm("/bin/init");
    assert(init_file != MM_NADDR);
    mm_pagedir_t pd = mm_pagedir_alloc();
    mm_clone(pd, mm_kernel);
    uintptr_t entry = elf_load(pd, init_file, 0);
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
    x86_mm_map(pd, 0x80000000, 0x800000, X86_MM_FLG_US | X86_MM_FLG_RW | X86_MM_FLG_PS);

    assert(x86_task_setup_stack(
        (struct x86_task *) task,
        (void(*)(void *)) entry,
        NULL,
        pd,
        ebp0,
        ebp3,
        0
    ) == 0);

    mm_dump_pages(pd);

    task_enable(task);

    // This is where we're ready to accept the first interrupt and start multitasking mode
    irq_enable();
    while (1) {
        __idle();
    }
}
