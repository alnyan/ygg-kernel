#include "intmacros.S"

.section .text

.macro irq_handler n
.extern x86_irq_handler_\n
.global x86_irq_\n
x86_irq_\n:
    cli

    x86_int_entry

    push %ebp
    call x86_irq_handler_\n
    pop %ebp

    mov sched_current, %esi
    test %esi, %esi
    jz 1f
    mov X86_TASK_ESP0(%esi), %esp

    // Set TSS entry to %esp + 18 * 4
    mov %esp, %ecx
    add $(18 * 4), %ecx
    mov %ecx, (X86_TSS_ESP0 + x86_tss)

1:  // Sched set task to null/Syscall didn't call sched()
    x86_irq_master_exit

    iret
.endm

irq_handler 1
irq_handler 8
irq_handler 11
irq_handler 14

#ifdef ENABLE_TASK
.global x86_irq_syscall
x86_irq_syscall:
    cli

    x86_int_entry

    push %esp
    call x86_syscall
    pop %esp

    // Check if syscall called sched()
    test %eax, %eax
    jz 1f

    // New task is in sched_current
    mov sched_current, %esi
    test %esi, %esi
    jz 1f
    mov X86_TASK_ESP0(%esi), %esp

    // Set TSS entry to %esp + 18 * 4
    mov %esp, %ecx
    add $(18 * 4), %ecx
    mov %ecx, (X86_TSS_ESP0 + x86_tss)

1:  // Sched set task to null/Syscall didn't call sched()
    x86_isr_exit

    iret
#endif
