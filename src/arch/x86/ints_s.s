.section .text
.extern x86_isr_handler

x86_isr_generic:
    pushal

    // Stack state:
    // ss
    // esp
    // eflags
    // cs
    // eip
    // eax
    // ecx
    // edx
    // ebx
    // oesp
    // ebp
    // esi
    // edi

    call x86_isr_handler

    popal

    addl $8, %esp
    iret

.global x86_irq_0
x86_irq_0:
    cli

    pushal

    // Stack state:
    // ss
    // esp
    // eflags
    // cs
    // eip
    // eax
    // ecx
    // edx
    // ebx
    // oesp
    // ebp
    // esi
    // edi

    // Push a pointer to all the registers on stack
    push %esp

    call x86_timer_handler

    // Remove pointer from stack
    addl $4, %esp

    movw $0x20, %dx
    movb $0x20, %al
    outb %al, %dx

    popal

    iret

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
x86_isr_stub    14
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
