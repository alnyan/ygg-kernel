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
// extern int x86_task_setup_stack(struct x86_task *t,
//     void (*entry)(void *),
//     void *arg,
//     mm_pagedir_t pd,
//     uint32_t ebp0,
//     uint32_t ebp3,
//     uint32_t ebp3p,
//     int flag);

// #define TASK_COPY_VIRT_BASE     0xF0000000
//
// static void task_copy_pages(mm_pagedir_t dst, const mm_pagedir_t src) {
//     mm_clone(dst, mm_kernel, MM_CLONE_FLG_KERNEL);
//
//     // Copy sections and stack
//     for (uint32_t pdi = 0; pdi < (KERNEL_VIRT_BASE - 1) >> 22; ++pdi) {
//         if (src[pdi] & X86_MM_FLG_PR) {
//             if (src[pdi] & X86_MM_FLG_PS) {
//                 // Copy 4M page
//                 uint32_t src_phys = src[pdi] & -MM_PAGESZ_HUGE;
//                 uint32_t dst_phys = mm_alloc_phys_page(MM_PAGESZ_HUGE);
//                 assert(dst_phys != MM_NADDR);
//
//                 assert(!(dst[pdi] & X86_MM_FLG_PR));
//
// #if defined(ENABLE_MAP_TRACE)
//                 kdebug("map %p[%d (%p)] = %p\n", dst, pdi, pdi << 22, dst_phys);
// #endif
//                 // Map it into dst
//                 dst[pdi] = dst_phys | (src[pdi] & 0x3FFFFF);
//
//                 assert(!(mm_kernel[TASK_COPY_VIRT_BASE >> 22] & X86_MM_FLG_PR));
//
//                 mm_map_page(mm_kernel, TASK_COPY_VIRT_BASE, dst_phys, MM_FLG_RW | MM_FLG_HUGE);
//                 mm_map_page(mm_kernel, TASK_COPY_VIRT_BASE + MM_PAGESZ_HUGE, src_phys, MM_FLG_HUGE);
//
//                 memcpy((void *) TASK_COPY_VIRT_BASE, (const void *) (TASK_COPY_VIRT_BASE + MM_PAGESZ_HUGE), MM_PAGESZ_HUGE);
//
//                 mm_unmap_cont_region(mm_kernel, TASK_COPY_VIRT_BASE, 1, MM_FLG_HUGE);
//                 mm_unmap_cont_region(mm_kernel, TASK_COPY_VIRT_BASE + MM_PAGESZ_HUGE, 1, MM_FLG_HUGE);
//             } else {
//                 // Copy 4K table
//                 mm_pagetab_t src_pt = (mm_pagetab_t) x86_mm_reverse_lookup(src[pdi] & -0x1000);
//                 assert((uintptr_t) src_pt != MM_NADDR);
//                 uintptr_t dst_pt_phys;
//                 mm_pagetab_t dst_pt = x86_mm_pagetab_alloc(dst, &dst_pt_phys);
//                 assert(dst_pt);
//
//                 assert(!(mm_kernel[TASK_COPY_VIRT_BASE >> 22] & X86_MM_FLG_PR));
//
//                 // Map it into dst
//                 assert(!(dst[pdi] & X86_MM_FLG_PR));
//                 dst[pdi] = dst_pt_phys | (src[pdi] & 0xFFF);
//
//                 for (uint32_t pti = 0; pti < 1024; ++pti) {
//                     if (src_pt[pti] & X86_MM_FLG_PR) {
//                         // Allocate 4K page
//                         uintptr_t src_phys = src_pt[pti] & -MM_PAGESZ_SMALL;
//                         uintptr_t dst_phys = mm_alloc_phys_page(MM_PAGESZ_SMALL);
//                         assert(dst_phys != MM_NADDR);
//
//                         // Map page into table
// #if defined(ENABLE_MAP_TRACE)
//                         kdebug("map %p[%d (%p)] = %p\n", dst_pt, pti, (pdi << 22) | (pti << 12), dst_phys);
// #endif
//                         dst_pt[pti] = dst_phys | (src_pt[pti] & 0xFFF) | X86_MM_FLG_PR;
//
//                         mm_map_page(mm_kernel, TASK_COPY_VIRT_BASE, dst_phys, MM_FLG_RW);
//                         mm_map_page(mm_kernel, TASK_COPY_VIRT_BASE + MM_PAGESZ_SMALL, src_phys, 0);
//
//                         memcpy((void *) TASK_COPY_VIRT_BASE, (const void *) (TASK_COPY_VIRT_BASE + MM_PAGESZ_SMALL), MM_PAGESZ_SMALL);
//
//                         mm_unmap_cont_region(mm_kernel, TASK_COPY_VIRT_BASE, 1, 0);
//                         mm_unmap_cont_region(mm_kernel, TASK_COPY_VIRT_BASE + MM_PAGESZ_SMALL, 1, 0);
//                     }
//                 }
//             }
//         }
//     }
// }

// task_t *task_fork(task_t *t) {
//     struct x86_task *src = (struct x86_task *) t;
//
//     uint32_t cr3_0;
//     asm volatile ("mov %%cr3, %0":"=a"(cr3_0));
//     assert(cr3_0 == (uint32_t) mm_kernel - KERNEL_VIRT_BASE);
//
//     // TODO: smarter cloning, don't just blindly clone all sub-kernel pages
//     //  * Probably use COW
//     //  * Use some markers to tell, for example, .text-pages from .data ones, as fork()s may share
//     //   the same code
//     uint32_t src_cr3 = *((uint32_t *) (src->ebp0 - 14 * 4));
//     mm_pagedir_t src_pd = (mm_pagedir_t) x86_mm_reverse_lookup(src_cr3);
//     assert((uintptr_t) src_pd != MM_NADDR);
//     mm_pagedir_t dst_pd = mm_pagedir_alloc(NULL);
//     assert(dst_pd);
//
//     // Data copying happens here
//     task_copy_pages(dst_pd, src_pd);
//
//     // Clone registers on stack
//     uint32_t dst_ebp0 = (uint32_t) heap_alloc(18 * 4) + 18 * 4;
//     memcpy((void *) (dst_ebp0 - 18 * 4), (const void *) (src->ebp0 - 18 * 4), 18 * 4);
//
//     // However, we need to replace cr3
//     uintptr_t dst_pd_phys = mm_lookup(mm_kernel, (uintptr_t) dst_pd, MM_FLG_HUGE, NULL);
//     assert(dst_pd_phys != MM_NADDR);
//     //*((uint32_t *) (dst_ebp0 - 14 * 4)) = (mm_kernel[(uintptr_t) dst_pd >> 22] & -0x400000) | ((uintptr_t) dst_pd & 0x3FFFFF);
//     *((uint32_t *) (dst_ebp0 - 14 * 4)) = dst_pd_phys;
//     // Fork returns 0
//     *((uint32_t *) (dst_ebp0 - 6 * 4)) = 0;
//
//     // Here comes new task
//     struct x86_task *dst = (struct x86_task *) task_create();
//     assert(dst);
//
//     // Setup stacks
//     dst->ebp0 = dst_ebp0;
//     dst->ebp3 = src->ebp3;
//     dst->esp0 = dst_ebp0 - 18 * 4;
//
//     // TODO: actually clone open descriptors
//     dst->ctl->fds[0] = vfs_open("/dev/tty0", VFS_FLG_WR);
//     dst->ctl->fds[1] = vfs_open("/dev/tty0", VFS_FLG_RD);
//
//     // Add it to sched
//     task_enable(dst);
//
//     return dst;
// }
//
// int task_execve(task_t *dst, const char *path, const char **argp, const char **envp) {
//     assert(!argp && !envp);     // These are not supported yet
//
//     uint32_t cr3_0;
//     asm volatile ("mov %%cr3, %0":"=a"(cr3_0));
//     assert(cr3_0 == (uint32_t) mm_kernel - KERNEL_VIRT_BASE);
//
//     // Task stack
//     uint32_t ebp0 = ((struct x86_task *) dst)->ebp0;
//     uint32_t ebp3 = ((struct x86_task *) dst)->ebp3;
//
//     // TODO: allow loading from sources other than ramdisk
//     uintptr_t file_mem = vfs_getm(path);
//     assert(file_mem != MM_NADDR);
//
//     // Pagedir
//     uint32_t cr3 = *((uint32_t *) (((struct x86_task *) dst)->ebp0 - 14 * 4));
//     mm_pagedir_t task_pd = (mm_pagedir_t) x86_mm_reverse_lookup(cr3);
//     assert((uintptr_t) task_pd != MM_NADDR);
//
//     uint32_t ebp3p = mm_lookup(task_pd, ebp3 - MM_PAGESZ_SMALL, 0, NULL);
//     assert(ebp3p != MM_NADDR);
//
//     // TODO: cleanup unused pages
//     uintptr_t entry = elf_load(task_pd, file_mem, 0);
//
//     assert(entry != MM_NADDR);
//
//     assert(x86_task_setup_stack(
//         (struct x86_task *) dst,
//         (void(*)(void *)) entry,
//         NULL,
//         task_pd,
//         ebp0,
//         ebp3,
//         ebp3p,
//         0
//     ) == 0);
//
//     return -1;
// }

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
    task->esp3_size = 4;

    // vfs_file_t *fd_tty_wr = vfs_open("/dev/tty0", VFS_FLG_WR);
    // assert(fd_tty_wr);
    // fd_tty_wr->task = task;
    // ((struct x86_task *) task)->ctl->fds[0] = fd_tty_wr;

    // vfs_file_t *fd_tty_rd = vfs_open("/dev/tty0", VFS_FLG_RD);
    // assert(fd_tty_rd);
    // fd_tty_rd->task = task;
    // ((struct x86_task *) task)->ctl->fds[1] = fd_tty_rd;

    assert(x86_task_set_context(task, entry, NULL, 0) == 0);

    task_enable(task);

    return task;
}
