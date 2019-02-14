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

// TODO: real allocator
static int s_lastStack = 0;
static int s_lastPagedir = 0;
static int s_lastThread = 0;
static uint32_t s_lastPid = 0;
static uint32_t s_pagedirs[1024 * X86_TASK_MAX] __attribute__((aligned(4096)));
static uint32_t s_stacks[X86_TASK_TOTAL_STACK * X86_TASK_MAX];
static struct x86_task s_taskStructs[sizeof(struct x86_task) * X86_TASK_MAX];
static struct x86_task_ctl s_taskCtls[sizeof(struct x86_task_ctl) * X86_TASK_MAX];

struct x86_task *x86_task_current = NULL;
struct x86_task *x86_task_first = NULL;
static struct x86_task *x86_task_idle = NULL;
static int x86_tasking_entry = 1;

extern void x86_task_idle_func(void *arg);

/*void task0(void *arg) {*/
    /*uint32_t myarg = (uint32_t) arg;*/
    /*uint16_t *myptr = (uint16_t *) (0xB8000 + (myarg & 0xFF));*/
    /*uint32_t sleep = (myarg >> 16) & 0xFF;*/
    /*int state = 1;*/

    /*while (1) {*/
        /*if (state) {*/
            /**myptr = 0x2000 | (((myarg >> 8) & 0xFF) - 'A' + 'a');*/
        /*} else {*/
            /**myptr = 0x0200 | ((myarg >> 8) & 0xFF);*/
        /*}*/

        /*state = !state;*/

        /*asm volatile("movl %0, %%eax; int $0x80"::"r"(sleep):"memory");*/
    /*}*/
/*}*/

struct x86_task *x86_task_alloc(int flag) {
    struct x86_task *t = &s_taskStructs[s_lastThread++];
    if (flag & X86_TASK_IDLE) {
        t->ctl = NULL;
    } else {
        t->ctl = &s_taskCtls[s_lastThread - 1];
    }

    t->flag = 0;
    t->next = NULL;
    return t;
}

int x86_alloc_pid(void) {
    return s_lastPid++;
}

int x86_task_setup_stack(struct x86_task *t, void (*entry)(void *), void *arg, mm_pagedir_t pd, uint32_t ebp3, int flag) {
    uint32_t stackIndex = s_lastStack++;
    uint32_t cr3 = ((uintptr_t) pd) - KERNEL_VIRT_BASE;
    uint32_t *esp0;
    uint32_t *esp3;

    if (!(flag & X86_TASK_IDLE)) {
        t->ebp3 = ebp3;
    }
    // Create kernel-space stack for state storage
    t->ebp0 = (uint32_t) &s_stacks[stackIndex * X86_TASK_TOTAL_STACK + X86_TASK_TOTAL_STACK];

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

    s_lastStack = 0;

    struct x86_task *prev_task = NULL;
    struct x86_task *task;

    // Create idle task (TODO: make it kernel space, so it can HLT)
    task = x86_task_alloc(X86_TASK_IDLE);
    task->next = NULL;
    task->flag = 0;
    task->pid = x86_alloc_pid();
    x86_task_setup_stack(task, x86_task_idle_func, NULL, mm_kernel, 0, X86_TASK_IDLE);

    x86_task_idle = task;
    x86_task_current = task;
    x86_task_first = task;

    prev_task = task;

    // TODO: create a user-space task here once I'm done with ELF loading and simple initrd
    // Find an ELF to load
    struct multiboot_mod_list *mod_list = (struct multiboot_mod_list *) (KERNEL_VIRT_BASE + x86_multiboot_info->mods_addr);
    debug("Multiboot provided kernel with %d modules\n", x86_multiboot_info->mods_count);

    if (x86_multiboot_info->mods_count != 1) {
        panic("Kernel expected 1 module, but there're %d\n", x86_multiboot_info->mods_count);
    }

    uintptr_t mod_base = KERNEL_VIRT_BASE + mod_list->mod_start;
    size_t mod_size = mod_list->mod_end - mod_list->mod_start;

    debug("Init module of %uK\n", mod_size / 1024);

    task = x86_task_alloc(0);
    task->next = NULL;
    task->flag = 0;
    task->pid = x86_alloc_pid();

    // Create a memory space
    mm_pagedir_t pd = &s_pagedirs[(s_lastPagedir++) * 1024];
    memset(pd, 0, 4096);
    pd[768] = X86_MM_FLG_PR | X86_MM_FLG_PS | X86_MM_FLG_RW;

    // For test, allow task to write video mem at 0xB8000 -> 0xD00B8000
    pd[(0xD0000000 >> 22)] = X86_MM_FLG_RW | X86_MM_FLG_PS | X86_MM_FLG_US | X86_MM_FLG_PR;
    // Userspace stack
    pd[(0x80000000 >> 22)] = (mm_alloc_phys_page() & -0x400000) | X86_MM_FLG_PR | X86_MM_FLG_US |
                                                                  X86_MM_FLG_PS | X86_MM_FLG_RW;

    uint32_t entry_addr;

    if ((entry_addr = elf_load(pd, mod_base, mod_size)) == MM_NADDR) {
        panic("Failed to load ELF\n");
    }

    mm_dump_pages(pd);
    // TODO: allocate an userspace stack for task
    x86_task_setup_stack(task, entry_addr, (void *) 0xD00B8000, pd, 0x80000000 + 0x400000, 0);

    prev_task->next = task;
}

void x86_task_switch(x86_irq_regs_t *regs) {
    if (x86_tasking_entry) {
        x86_tasking_entry = 0;
        return;
    }

    // Update tasks' flags and ctl
    for (struct x86_task *t = x86_task_first; t; t = t->next) {
        if (t == x86_task_idle) {
            continue;
        }
        // Decrease sleep counters
        if (t->flag & TASK_FLG_WAIT) {
            if (!(--t->ctl->sleep)) {
                t->flag &= ~TASK_FLG_WAIT;
            }
        }
    }

    struct x86_task *from = x86_task_current;

    x86_task_current->flag &= ~TASK_FLG_RUNNING;

    while (1) {
        if (!(x86_task_current = x86_task_current->next)) {
            x86_task_current = x86_task_first;
        }

        if (x86_task_current->flag & TASK_FLG_WAIT) {
            // If a loop was done and we couldn't find any task, just use the previous one
            if (x86_task_current == from) {
                break;
            }
            continue;
        }

        break;
    }

    if (x86_task_current == from && (x86_task_current->flag & TASK_FLG_WAIT)) {
        // If we couldn't find any non-waiting task
        x86_task_current = x86_task_idle;
        x86_task_current->flag |= TASK_FLG_RUNNING;
        return;
    }

    x86_task_current->flag |= TASK_FLG_RUNNING;
}
