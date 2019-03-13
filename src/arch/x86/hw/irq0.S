#include "intmacros.S"

.section .text
.global x86_irq_0

.extern x86_timer_func
.extern x86_tss

x86_irq_0:
    cli

    x86_int_entry

    ///// <timer-specific things>

    // Call timer handler (if set)
    movl x86_timer_func, %eax
    test %eax, %eax
    jz 1f
    call *%eax

1:  // No timer func
    call sched

    // New task is in sched_current
    mov sched_current, %esi
    test %esi, %esi
    jz 1f
    mov X86_TASK_ESP0(%esi), %esp

    // Set TSS entry to %esp + 18 * 4
    mov %esp, %ecx
    add $(18 * 4), %ecx
    mov %ecx, (X86_TSS_ESP0 + x86_tss)

1:  // Sched set task to null

    //// </timer-specific things>

    x86_irq_master_exit

    iret
