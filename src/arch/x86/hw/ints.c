#include "ints.h"
#include <stdint.h>
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/assert.h"
#include "sys/sched.h"
#include <uapi/signum.h>
#include "irq.h"

typedef struct {
    uint16_t base_lo;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_hi;
} x86_idt_entry_t;

typedef struct {
    uint16_t size;
    uint32_t offset;
} __attribute__((packed)) x86_idt_ptr_t;

#define IDT_NENTR   256

#define IDT_FLG_TASK32      0x5
#define IDT_FLG_INT16       0x6
#define IDT_FLG_TRAP16      0x7
#define IDT_FLG_INT32       0xE
#define IDT_FLG_TRAP32      0xF
#define IDT_FLG_SS          (1 << 4)
#define IDT_FLG_R0          (0 << 5)
#define IDT_FLG_R1          (1 << 5)
#define IDT_FLG_R2          (2 << 5)
#define IDT_FLG_R3          (3 << 5)
#define IDT_FLG_P           (1 << 7)

static const char *s_error_text[] = {
    "division by zero",
    "debug",
    "non-maskable interrupt",
    "breakpoint",
    "overflow",
    "bound range exceeded",
    "invalid opcode",
    "device not available",
    "double fault",
    NULL,
    "invalid tss",
    "segment not present",
    "stack segment fault",
    "general protection fault",
    "page fault",                   // Handled separately
    NULL,
    "x87 FPU error",
    "alignment error",
    "machine error",
    "SIMD FPU error",
    "virtualization error",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "security error",
    NULL,
};

static x86_idt_entry_t s_idt[IDT_NENTR];
static x86_idt_ptr_t s_idtr;

extern void x86_isr_0();
extern void x86_isr_1();
extern void x86_isr_2();
extern void x86_isr_3();
extern void x86_isr_4();
extern void x86_isr_5();
extern void x86_isr_6();
extern void x86_isr_7();
extern void x86_isr_8();
extern void x86_isr_9();
extern void x86_isr_10();
extern void x86_isr_11();
extern void x86_isr_12();
extern void x86_isr_13();
extern void x86_isr_14();
extern void x86_isr_15();
extern void x86_isr_16();
extern void x86_isr_17();
extern void x86_isr_18();
extern void x86_isr_19();
extern void x86_isr_20();
extern void x86_isr_21();
extern void x86_isr_22();
extern void x86_isr_23();
extern void x86_isr_24();
extern void x86_isr_25();
extern void x86_isr_26();
extern void x86_isr_27();
extern void x86_isr_28();
extern void x86_isr_29();
extern void x86_isr_30();
extern void x86_isr_31();

// "Magic" timer IRQ #0
extern void x86_irq_0();


// Generic IRQs like keyboard
extern void x86_irq_1();
extern void x86_irq_2();
extern void x86_irq_3();
extern void x86_irq_4();
extern void x86_irq_5();
extern void x86_irq_6();
extern void x86_irq_7();
extern void x86_irq_8();
extern void x86_irq_9();
extern void x86_irq_10();
extern void x86_irq_11();
extern void x86_irq_12();
extern void x86_irq_13();
extern void x86_irq_14();
extern void x86_irq_15();

// System call IRQ
extern void x86_irq_syscall();
extern uint32_t x86_error_num;

void x86_isr_handler(x86_irq_regs_t *regs) {
    if (regs->iret.cs == 0x08) {
        kfatal("Kernel error: %s\n", s_error_text[x86_error_num % 32]);

        panic("Non-recoverable kernel error\n");
    } else {
        // TODO: better dump
        assert(sched_current);
        kinfo("[%d] error: %s\n", task_ctl(sched_current)->pid, s_error_text[x86_error_num % 32]);

        // Terminate the offender task
        task_terminate(sched_current, SIGSEGV);
        sched();
    }
}

void x86_idt_set(int idx, uint32_t base, uint16_t selector, uint8_t flags) {
    s_idt[idx].base_lo = base & 0xFFFF;
    s_idt[idx].base_hi = (base >> 16) & 0xFFFF;
    s_idt[idx].selector = selector;
    s_idt[idx].flags = flags;
    s_idt[idx].zero = 0;
}

void ints_init(void) {
    kdebug("Setting up IDT entries\n");

    x86_idt_set(0, (uint32_t) x86_isr_0, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(1, (uint32_t) x86_isr_1, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(2, (uint32_t) x86_isr_2, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(3, (uint32_t) x86_isr_3, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(4, (uint32_t) x86_isr_4, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(5, (uint32_t) x86_isr_5, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(6, (uint32_t) x86_isr_6, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(7, (uint32_t) x86_isr_7, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(8, (uint32_t) x86_isr_8, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(9, (uint32_t) x86_isr_9, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(10, (uint32_t) x86_isr_10, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(11, (uint32_t) x86_isr_11, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(12, (uint32_t) x86_isr_12, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(13, (uint32_t) x86_isr_13, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(14, (uint32_t) x86_isr_14, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(15, (uint32_t) x86_isr_15, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(16, (uint32_t) x86_isr_16, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(17, (uint32_t) x86_isr_17, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(18, (uint32_t) x86_isr_18, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(19, (uint32_t) x86_isr_19, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(20, (uint32_t) x86_isr_20, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(21, (uint32_t) x86_isr_21, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(22, (uint32_t) x86_isr_22, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(23, (uint32_t) x86_isr_23, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(24, (uint32_t) x86_isr_24, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(25, (uint32_t) x86_isr_25, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(26, (uint32_t) x86_isr_26, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(27, (uint32_t) x86_isr_27, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(28, (uint32_t) x86_isr_28, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(29, (uint32_t) x86_isr_29, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(30, (uint32_t) x86_isr_30, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(31, (uint32_t) x86_isr_31, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);

    pic8259_init();

    x86_idt_set(32, (uint32_t) x86_irq_0, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);

    x86_idt_set(33, (uint32_t) x86_irq_1, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    /*x86_idt_set(34, (uint32_t) x86_irq_2, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);*/
    /*x86_idt_set(35, (uint32_t) x86_irq_3, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);*/
    /*x86_idt_set(36, (uint32_t) x86_irq_4, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);*/
    /*x86_idt_set(37, (uint32_t) x86_irq_5, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);*/
    /*x86_idt_set(38, (uint32_t) x86_irq_6, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);*/
    x86_idt_set(40, (uint32_t) x86_irq_8, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(43, (uint32_t) x86_irq_11, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);
    x86_idt_set(46, (uint32_t) x86_irq_14, 0x08, IDT_FLG_P | IDT_FLG_R0 | IDT_FLG_INT32);

#if defined(ENABLE_TASK)
    x86_idt_set(128, (uint32_t) x86_irq_syscall, 0x08, IDT_FLG_P | IDT_FLG_R3 | IDT_FLG_INT32);
#endif

    s_idtr.offset = (uint32_t) s_idt;
    s_idtr.size = sizeof(s_idt) - 1;

    asm volatile ("lidt (s_idtr)":::"memory");
}
