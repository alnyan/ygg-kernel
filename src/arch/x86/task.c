#include "task.h"
#include "arch/hw.h"
#include <stddef.h>
#include "hw.h"
#include "multiboot.h"
#include "sys/panic.h"
#include "sys/task.h"
#include "sys/elf.h"
#include "sys/debug.h"
#include "sys/mm.h"
#include "sys/mem.h"

void task_busy(void *task) {
    ((struct x86_task *) task)->flag |= TASK_FLG_BUSY;
}

void task_nobusy(void *task) {
    ((struct x86_task *) task)->flag &= ~TASK_FLG_BUSY;
}

// TODO: real allocator
static struct x86_task x86_task_idle;
static uint32_t x86_task_idle_stack[X86_TASK_TOTAL_STACK];

struct x86_task *x86_task_current = NULL;
struct x86_task *x86_task_first = NULL;

static int x86_tasking_entry = 1;

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

    // Create idle task (TODO: make it kernel space, so it can HLT)
    x86_task_idle.next = NULL;
    x86_task_idle.ctl = NULL;
    x86_task_idle.flag = 0;
    x86_task_idle.pid = 0;
    x86_task_setup_stack(&x86_task_idle,
            x86_task_idle_func,
            NULL,
            mm_kernel,
            (uint32_t) &x86_task_idle_stack[X86_TASK_TOTAL_STACK],
            0,
            X86_TASK_IDLE);

    x86_task_current = &x86_task_idle;
    x86_task_first = &x86_task_idle;
}

void x86_task_switch(x86_irq_regs_t *regs) {
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
            if (tp) {
                tp->next = t->next;
            }
            debug("Task %d exited with status %d\n",
                    t->pid,
                    *((uint32_t *) (t->ebp0 - 8 * 4)));

            // Attempted to kill root process
            if (t->pid == 1) {
                panic_irq("Attempted to kill init!\n", regs);
            }
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
