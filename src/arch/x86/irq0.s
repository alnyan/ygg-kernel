.section .text
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

    // TODO: add code here to call scheduler

    // Remove pointer from stack
    addl $4, %esp

    movw $0x20, %dx
    movb $0x20, %al
    outb %al, %dx

    popal

    iret

