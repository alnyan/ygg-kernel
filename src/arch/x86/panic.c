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

    kfatal("--- Register dump ---\n");
    x86_dump_gp_regs(DEBUG_FATAL, &regs->gp);
    x86_dump_iret_regs(DEBUG_FATAL, &regs->iret);

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
    x86_dump_gp_regs(DEBUG_FATAL, &regs->gp);
    x86_dump_iret_regs(DEBUG_FATAL, &regs->iret);

    panic_hlt();
}
