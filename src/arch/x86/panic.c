#include "sys/panic.h"
#include "sys/debug.h"
#include "mm.h"
#include "arch/hw.h"
#include "sys/mm.h"
#include "task/task.h"

#define X86_PF_FLG_PR   (1 << 0)
#define X86_PF_FLG_RW   (1 << 1)
#define X86_PF_FLG_US   (1 << 2)
#define X86_PF_FLG_ID   (1 << 4)

#if defined(ENABLE_KERNEL_MAP)
#include "sys/vfs.h"
#include "sys/string.h"

static uint32_t x86_parse_addr(const char *text) {
    uint32_t r = 0;

    while (*text && *text != ' ') {
        r *= 16;

        if (*text >= 'a' && *text <= 'f') {
            r += *text - 'a' + 10;
        } else if (*text >= '0' && *text <= '9') {
            r += *text - '0';
        }
        ++text;
    }

    return r;
}

static int x86_find_func_name(uint32_t addr, char *name, uint32_t *off) {
    vfs_file_t *f = vfs_open("/etc/kernel.map", VFS_FLG_RD);
    if (!f) {
        return -1;
    }

    char linebuf[255];
    char namebuf[255];
    uint32_t prev_addr = 0xFFFFFFFF;
    const char *target_name = NULL;

    while (vfs_gets(f, linebuf, sizeof(linebuf)) > 0) {
        if (!strncmp(linebuf, "0x", 2)) {
            uint32_t a = x86_parse_addr(linebuf + 2);

            if (addr < a) {
                break;
            }

            strcpy(namebuf, linebuf + 13);
            target_name = namebuf;
            prev_addr = a;
            *off = addr - prev_addr;
        }
    }

    vfs_close(f);

    if (target_name) {
        strcpy(name, target_name);
    }

    return target_name ? 0 : -1;
}

static void x86_crash_backtrace(uint32_t eeip, const uint32_t *stack, int depth) {
    char namebuf[255];
    uint32_t off;

    kinfo(" @ %p\n", eeip);
    if (x86_find_func_name(eeip, namebuf, &off) == 0) {
        kinfo("   <%s + %u>\n", namebuf, off);
    }

    while (depth) {
        uint32_t ebp = stack[0];
        uint32_t eip = stack[1];

        kinfo(" %d: %p\n", depth, eip);
        if (x86_find_func_name(eip, namebuf, &off) == 0) {
            kinfo("   <%s + %u>\n", namebuf, off);
        }

        stack = (uint32_t *) ebp;
        --depth;
    }
}

#endif

void panic_reg(void) {
    if (x86_task_current) {
        kfatal("Offender task:\n");
        x86_task_dump_context(DEBUG_FATAL, x86_task_current);
    }
}

// Implements IRQ and ISR-specific panics, which dump registers
void panicf_isr(const char *fmt, const x86_int_regs_t *regs, ...) {
    kfatal(PANIC_MSG_INTRO);
    va_list args;
    va_start(args, regs);
    debugfv(DEBUG_FATAL, fmt, args);
    va_end(args);

    kfatal("Exception code: #%u\n", regs->int_no);
    kfatal("Error code: %d (0x%x)\n", regs->err_code);

    if (x86_task_current) {
        kfatal("Offender task:\n");
        x86_task_dump_context(DEBUG_FATAL, x86_task_current);
    }

    if (regs->int_no == 14) {
        uint32_t cr2;
        asm volatile ("movl %%cr2, %0":"=a"(cr2));

        kfatal("--- Page fault ---\n");

        kfatal("Flags: %c%c%c%c\n",
                (regs->err_code & X86_PF_FLG_PR) ? 'P' : '-',
                (regs->err_code & X86_PF_FLG_RW) ? 'W' : 'R',
                (regs->err_code & X86_PF_FLG_US) ? 'U' : '-',
                (regs->err_code & X86_PF_FLG_ID) ? 'I' : 'D');
        kfatal("CR2 = %p\n", cr2);

        uint32_t cr3;
        asm volatile ("movl %%cr3, %0":"=a"(cr3));

        kfatal("CR3 = %p\n", cr3);
        // if (cr3 == (uintptr_t) mm_kernel - KERNEL_VIRT_BASE) {
        //     kfatal("\t(Is kernel)\n");

        //     uint32_t pde = mm_kernel[cr2 >> 22];
        //     if (pde & 1) {
        //         kfatal("\tEntry for CR2 is present in PD\n");
        //     } else {
        //         kfatal("\tEntry is not mapped in kernel PD\n");
        //     }
        // } else {
        //     kfatal("\tOffender task PID: %u\n", x86_task_current->ctl->pid);
        // }

        // if (cr3 == (uintptr_t) mm_kernel - KERNEL_VIRT_BASE) {
        //     kfatal("---- Page directory ----\n");
        //     mm_dump_pages(mm_kernel);
        // } else {
        //     mm_pagedir_t pd = (mm_pagedir_t) x86_mm_reverse_lookup(cr3);

        //     if ((uintptr_t) pd != MM_NADDR) {
        //         uintptr_t phys_addr = MM_NADDR;

        //         if (pd[cr2 >> 22] & X86_PF_FLG_PR) {
        //             if (pd[cr2 >> 22] & X86_MM_FLG_PS) {
        //                 phys_addr = (pd[cr2 >> 22] & -0x400000) | (cr2 & 0x3FFFFF);
        //             } else {
        //                 mm_pagetab_t pt = (mm_pagetab_t) x86_mm_reverse_lookup(pd[cr2 >> 22] & -0x1000);

        //                 if ((uintptr_t) pt != MM_NADDR) {
        //                     phys_addr = (pt[(cr2 >> 12) & 0x3FF] & -0x1000) | (cr2 & 0xFFF);
        //                 }
        //             }
        //         }

        //         if (phys_addr == MM_NADDR) {
        //             kfatal("\tFault address is not mapped in task's PD\n");
        //         } else {
        //             kfatal("\tFault physical address is %p\n", phys_addr);
        //         }

        //         kfatal("---- Page directory ----\n");
        //         mm_dump_pages(pd);
        //     } else {
        //         kfatal("\tFailed to find virtual address of task's page directory\n");
        //     }
        // }
    }

    kfatal("--- Register dump ---\n");
    x86_dump_gp_regs(&regs->gp);
    x86_dump_iret_regs(&regs->iret);

#if defined(ENABLE_KERNEL_MAP)
    if (regs->iret.cs == 0x08) {
        extern uint32_t x86_crash_esp;
        uint32_t *stack = (uint32_t *) (x86_crash_esp);
        x86_crash_backtrace(regs->iret.eip, stack, 5);
    }
#endif

    panic_hlt();
}

void panicf_irq(const char *fmt, const x86_irq_regs_t *regs, ...) {
    kfatal(PANIC_MSG_INTRO);
    va_list args;
    va_start(args, regs);
    debugfv(DEBUG_FATAL, fmt, args);
    va_end(args);

    kfatal("--- Register dump ---\n");
    x86_dump_gp_regs(&regs->gp);
    x86_dump_iret_regs(&regs->iret);

    panic_hlt();
}
