.section .text

.set X86_INT_STACK, (256 * 4)
.extern x86_int_stack

.macro irq_handler n
.extern x86_irq_handler_\n
.global x86_irq_\n
x86_irq_\n:
    // Store current task's context
    pushal

    movl %cr3, %eax
    pushl %eax

    pushl %gs
    pushl %fs
    pushl %es
    pushl %ds

    // Switch to kernel interrupt stack
    movl %esp, %esi
    movl $(x86_int_stack + X86_INT_STACK), %esp

    // Preserve context stack ptr
    pushl %esi

    addl $4, %esi
    // Call IRQ handler
    pushl %esi
    call x86_irq_handler_\n
    addl $4, %esp

    // TODO: if current task's execution is blocked by a system call, just switch to some other task
.nosched:
    popl %esi
    movl %esi, %esp

    popl %eax
    movl %eax, %ds
    popl %eax
    movl %eax, %es
    popl %eax
    movl %eax, %fs
    popl %eax
    movl %eax, %gs

    popl %eax
    movl %eax, %cr3

    popal

    iretl
.endm

irq_handler 1
