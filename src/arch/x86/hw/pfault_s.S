#include "intmacros.S"

.section .text
.global x86_isr_14
.extern x86_tss

x86_isr_14:
    cli
    add $4, %esp

    x86_int_entry

    push %esp
    call x86_page_fault
    add $4, %esp


    // New task is in sched_current
    mov sched_current, %esi
    test %esi, %esi
    jz .fatal
    mov X86_TASK_ESP0(%esi), %esp

    // Set TSS entry to %esp + 18 * 4
    mov %esp, %ecx
    add $(18 * 4), %ecx
    mov %ecx, (X86_TSS_ESP0 + x86_tss)

    x86_isr_exit

    iret
.fatal:
1:
    hlt
    jmp 1b
