#include "task.h"
#include "arch/hw.h"
#include <stddef.h>
#include "../hw/hw.h"
#include "../multiboot.h"
#include "sys/panic.h"
#include "sys/task.h"
#include "sys/elf.h"
#include "sys/heap.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "sys/attr.h"
#include "sys/mm.h"
#include "sys/mem.h"

task_t *task_create(void) {
    struct x86_task *t = (struct x86_task *) heap_alloc(sizeof(struct x86_task));
    memset(t, 0, sizeof(struct x86_task *));
    t->ctl = task_ctl_create();

    return (task_t *) t;
}

void task_destroy(task_t *t) {
    struct x86_task *task = (struct x86_task *) t;
    uint32_t cr3_kernel = (uint32_t) mm_kernel - KERNEL_VIRT_BASE;
    asm volatile ("mov %0, %%cr3"::"a"(cr3_kernel));

    // Unmapping sub-kernel pages
    mm_pagedir_t task_pd;
    uint32_t cr3 = *((uint32_t *) (task->ebp0 - 14 * 4));
    task_pd = (mm_pagedir_t) (cr3 + KERNEL_VIRT_BASE);

    for (uint32_t i = 0; i < (KERNEL_VIRT_BASE >> 22) - 1; ++i) {
        if (task_pd[i] & 1) {
            mm_unmap_cont_region(task_pd, i << 22, 1, MM_UFLG_PF);
        }
    }

    // Close file descriptors
    for (int i = 0; i < 4; ++i) {
        if (task->ctl->fds[i]) {
            vfs_close(task->ctl->fds[i]);
        }
    }

    // Free stacks
    heap_free((void *) (task->ebp0 - 18 * 4));

    // Free pagedir
    //memset(task_pd, 0, 4096);
    mm_pagedir_free(task_pd);

    // Free data structures
    task_ctl_free(task->ctl);
    heap_free(task);
}

void task_busy(void *task) {
    ((struct x86_task *) task)->flag |= TASK_FLG_BUSY;
}

void task_nobusy(void *task) {
    ((struct x86_task *) task)->flag &= ~TASK_FLG_BUSY;
}

void task_copy_to_user(task_t *task, userspace void *dst, const void *src, size_t sz) {
    // Temp: just check we're entering here from kernel
    uint32_t cr3_0;
    asm volatile ("mov %%cr3, %0":"=a"(cr3_0));
    assert(cr3_0 == (uint32_t) mm_kernel - KERNEL_VIRT_BASE);

    assert(task);
    struct x86_task *t = (struct x86_task *) task;
    uint32_t cr3 = *((uint32_t *) (t->ebp0 - 14 * 4));
    mm_pagedir_t task_pd = (mm_pagedir_t) (cr3 + KERNEL_VIRT_BASE);

    uint32_t dst_page_base = ((uint32_t) dst) & -0x400000;

    // TODO: handle multiple-page copies
    // TODO: make standard define for "transition page"
    x86_mm_map(mm_kernel, dst_page_base, task_pd[dst_page_base >> 22], X86_MM_FLG_PS | X86_MM_FLG_RW);
    memcpy(dst, src, sz);
    mm_unmap_cont_region(mm_kernel, dst_page_base, 1, 0);
}

void task_copy_from_user(task_t *task, void *dst, const userspace void *src, size_t sz) {
    // Temp: just check we're entering here from kernel
    uint32_t cr3_0;
    asm volatile ("mov %%cr3, %0":"=a"(cr3_0));
    assert(cr3_0 == (uint32_t) mm_kernel - KERNEL_VIRT_BASE);

    assert(task);
    struct x86_task *t = (struct x86_task *) task;
    uint32_t cr3 = *((uint32_t *) (t->ebp0 - 14 * 4));
    mm_pagedir_t task_pd = (mm_pagedir_t) (cr3 + KERNEL_VIRT_BASE);

    uint32_t src_page_base = ((uint32_t) src) & -0x400000;

    // TODO: handle multiple-page copies
    // TODO: make standard define for "transition page"
    x86_mm_map(mm_kernel, src_page_base, task_pd[src_page_base >> 22], X86_MM_FLG_PS);
    if (sz != MM_NADDR) {
        memcpy(dst, src, sz);
    } else {
        const char *src_str = (const char *) src;
        char *dst_str = (char *) dst;

        for (; *src_str; ++src_str, ++dst_str) {
            *dst_str = *src_str;
        }
    }
    mm_unmap_cont_region(mm_kernel, src_page_base, 1, 0);
}

// Idle task (kernel-space) stuff
static struct x86_task x86_task_idle;
static uint32_t x86_task_idle_stack[X86_TASK_TOTAL_STACK];

// Scheduling stuff
struct x86_task *x86_task_current = NULL;
struct x86_task *x86_task_first = NULL;
struct x86_task *x86_task_last = NULL;

int x86_last_pid = 0;

void task_enable(task_t *t) {
    debug("Adding task %p to sched\n", t);

    struct x86_task *task = (struct x86_task *) t;

    // FIXME: move to platform-generic
    task->ctl->pid = ++x86_last_pid;
    task->flag = 0;

    x86_task_last->next = t;
    x86_task_last = t;
}

// If set to 1, means we haven't entered multitasking yet
static int x86_tasking_entry = 1;

// Assembly function which uses no stack and just loops with sti; hlt;
extern void x86_task_idle_func(void *arg);

int x86_task_setup_stack(struct x86_task *t,
        void (*entry)(void *),
        void *arg,
        mm_pagedir_t pd,
        uint32_t ebp0,
        uint32_t ebp3,
        int flag) {
    uint32_t cr3 = ((uintptr_t) pd) - KERNEL_VIRT_BASE;
    uint32_t *esp0;
    uint32_t *esp3;

    if (!(flag & X86_TASK_IDLE)) {
        t->ebp3 = ebp3;
    }
    // Create kernel-space stack for state storage
    t->ebp0 = ebp0;
    esp0 = (uint32_t *) t->ebp0;

    if (flag & X86_TASK_IDLE) {
        // Init kernel idle task
        *--esp0 = 0x0;              // No SS3 for idle task
        *--esp0 = 0x0;              // No ESP3 for idle task
        *--esp0 = 0x248;            // EFLAGS
        *--esp0 = 0x08;             // CS
    } else {
        // Init userspace task
        // Map stack page
        x86_mm_map(mm_kernel, ebp3 - 0x400000, pd[(ebp3 - 0x400000) >> 22] & -0x400000, X86_MM_FLG_RW | X86_MM_FLG_PS);
        esp3 = (uint32_t *) t->ebp3;

        *--esp3 = (uint32_t) arg;   // Push thread arg
        *--esp3 = 0x12345678;       // Push some funny return address

        mm_unmap_cont_region(mm_kernel, ebp3 - 0x400000, 1, 0);

        *--esp0 = 0x23;             // SS
        *--esp0 = (uint32_t) esp3;  // ESP
        *--esp0 = 0x248;            // EFLAGS
        *--esp0 = 0x1B;             // CS
    }
    *--esp0 = (uint32_t) entry; // EIP

    // Push GP regs
    for (int i = 0; i < 8; ++i) {
        *--esp0 = 0;
    }

    *--esp0 = cr3;              // CR3

    // Push segs
    for (int i = 0; i < 4; ++i) {
        *--esp0 = (flag & X86_TASK_IDLE) ? 0x10 : 0x23;
    }

    t->esp0 = (uint32_t) esp0;

    return 0;
}

void x86_task_init(void) {
    debug("Initializing multitasking\n");

    x86_task_idle.next = NULL;
    x86_task_idle.ctl = NULL;
    x86_task_idle.flag = 0;
    x86_task_setup_stack(&x86_task_idle,
            x86_task_idle_func,
            NULL,
            mm_kernel,
            (uint32_t) &x86_task_idle_stack[X86_TASK_TOTAL_STACK],
            0,
            X86_TASK_IDLE);

    x86_task_current = &x86_task_idle;
    x86_task_first = &x86_task_idle;
    x86_task_last = &x86_task_idle;
}

void x86_task_switch(x86_irq_regs_t *regs) {
    mm_set_kernel();

    if (x86_tasking_entry) {
        x86_tasking_entry = 0;
        return;
    }

    struct x86_task *tp = NULL;
    // Update tasks' flags and ctl
    for (struct x86_task *t = x86_task_first; t; t = t->next) {
        if (t == &x86_task_idle) {
            tp = t;
            continue;
        }
        // Task stopped
        if (t->flag & TASK_FLG_STOP) {
            // Remove task from sched here
            assert(tp);

            tp->next = t->next;
            if (t == x86_task_last) {
                x86_task_last = tp;
            }

            debug("Task %d exited with status %d\n",
                    t->ctl->pid,
                    *((uint32_t *) (t->ebp0 - 8 * 4)));

            // Attempted to kill root process
            if (t->ctl->pid == 1) {
                panic_irq("Attempted to kill init!\n", regs);
            }

            task_destroy(t);

            continue;
        }
        // Decrease sleep counters
        if (t->flag & TASK_FLG_WAIT) {
            if (!(--t->ctl->sleep)) {
                t->flag &= ~TASK_FLG_WAIT;
            }
        }
        tp = t;
    }

    struct x86_task *from = x86_task_current;

    x86_task_current->flag &= ~TASK_FLG_RUNNING;

    while (1) {
        if (!(x86_task_current = x86_task_current->next)) {
            x86_task_current = x86_task_first;
        }

        if ((x86_task_current->flag & TASK_FLG_WAIT) || (x86_task_current->flag & TASK_FLG_BUSY)) {
            // If a loop was done and we couldn't find any task, just use the previous one
            if (x86_task_current == from) {
                break;
            }
            continue;
        }

        break;
    }

    if (x86_task_current == from &&
            ((x86_task_current->flag & TASK_FLG_WAIT) ||
             (x86_task_current->flag & TASK_FLG_BUSY))) {
        // If we couldn't find any non-waiting task
        x86_task_current = &x86_task_idle;
        x86_task_current->flag |= TASK_FLG_RUNNING;
        return;
    }

    x86_task_current->flag |= TASK_FLG_RUNNING;
}
