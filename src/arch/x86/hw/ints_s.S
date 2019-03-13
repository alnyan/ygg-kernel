#include "intmacros.S"

.section .bss
.global x86_crash_esp
.global x86_error_num
x86_crash_esp:
    .skip 4
x86_error_num:
    .skip 4

.section .text

x86_isr_generic:
    cli

    // ///Magic///
    push %eax
    mov 4(%esp), %eax
    mov %eax, x86_error_num
    pop %eax
    add $8, %esp
    //

    x86_int_entry

    push %esp
    call x86_isr_handler
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

.macro x86_isr_stub_nc n
.global x86_isr_\n
x86_isr_\n:
    cli
    pushl $0 // Compensate for missing errcode
    pushl $\n
    jmp x86_isr_generic
.endm

.macro x86_isr_stub n
.global x86_isr_\n
x86_isr_\n:
    cli
    pushl $\n
    jmp x86_isr_generic
.endm

x86_isr_stub_nc 0
x86_isr_stub_nc 1
x86_isr_stub_nc 2
x86_isr_stub_nc 3
x86_isr_stub_nc 4
x86_isr_stub_nc 5
x86_isr_stub_nc 6
x86_isr_stub_nc 7
x86_isr_stub    8
x86_isr_stub_nc 9
x86_isr_stub    10
x86_isr_stub    11
x86_isr_stub    12
x86_isr_stub    13
x86_isr_stub_nc 15
x86_isr_stub_nc 16
x86_isr_stub    17
x86_isr_stub_nc 18
x86_isr_stub_nc 19
x86_isr_stub_nc 20
x86_isr_stub_nc 21
x86_isr_stub_nc 22
x86_isr_stub_nc 23
x86_isr_stub_nc 24
x86_isr_stub_nc 25
x86_isr_stub_nc 26
x86_isr_stub_nc 27
x86_isr_stub_nc 28
x86_isr_stub_nc 29
x86_isr_stub    30
x86_isr_stub_nc 31

.section .bss
x86_kernel_exc_stack:
    .skip 1024
x86_kernel_exc_stack_end:
