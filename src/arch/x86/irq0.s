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
    // gs
    // fs
    // es
    // ds

    pushl %gs
    pushl %fs
    pushl %es
    pushl %ds

    // Push a pointer to all the registers on stack
    push %esp

    // Setup kernel segments
    movl $0x10, %eax
    movl %ax, %ds
    movl %ax, %es
    movl %ax, %fs
    movl %ax, %gs

    call x86_irq_timer

    // New esp0 is returned by the call
    movl %eax, %esp

    // Remove pointer from stack
    popl %eax
    movl %eax, %ds
    popl %eax
    movl %eax, %es
    popl %eax
    movl %eax, %fs
    popl %eax
    movl %eax, %gs

    movw $0x20, %dx
    movb $0x20, %al
    outb %al, %dx

    popal

    iret

