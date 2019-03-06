.section .text

.set TSS_ESP0,              0x04

.set X86_TASK_STRUCT_ESP0,  0x00
.set X86_TASK_STACK,        18

.extern x86_int_stack

.macro irq_handler n
.extern x86_irq_handler_\n
.global x86_irq_\n
x86_irq_\n:
    // Store current task's context
    pushal

#ifdef ENABLE_TASK
    movl %cr3, %eax
    pushl %eax

    pushl %gs
    pushl %fs
    pushl %es
    pushl %ds

    movl $0x10, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs
    movl %eax, %gs

    // Switch to kernel interrupt stack
    movl %esp, %esi
    movl $x86_int_stack, %esp

    // Preserve context stack ptr
    pushl %esi

    // Call IRQ handler
    pushl %esi
    call x86_irq_handler_\n
    addl $4, %esp

    // TODO: if current task's execution is blocked by a system call, just switch to some other task
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
#else
    pushl mm_kernel
    pushl %gs
    pushl %es
    pushl %fs
    pushl %ds

    pushl %esp
    call x86_irq_handler_\n
    addl $4, %esp

    addl $16, %esp
    addl $4, %esp
#endif

    popal

    iretl
.endm

irq_handler 1
irq_handler 8
irq_handler 11
irq_handler 14

#ifdef ENABLE_TASK
.global x86_irq_syscall
x86_irq_syscall:
    // Store current task's context
    pushal

    movl %cr3, %eax
    pushl %eax

    pushl %gs
    pushl %fs
    pushl %es
    pushl %ds

    movl $0x10, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs
    movl %eax, %gs

    // Switch to kernel interrupt stack
    movl %esp, %esi
    movl $x86_int_stack, %esp

    pushl %esi
    call x86_syscall
    addl $4, %esp

    movl x86_task_current, %esi

    // Load TO task's esp0 to %eax and %edx
    movl X86_TASK_STRUCT_ESP0(%esi), %eax
    movl %eax, %edx

    // Write TSS entry, eax + stack size
    addl $(X86_TASK_STACK * 4), %edx
    movl %edx, x86_tss + TSS_ESP0

    // Now, esp should be TO task's esp0
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

    popl %eax
    movl %eax, %cr3

    movw $0x20, %dx
    movb $0x20, %al
    outb %al, %dx

    popal

    iret
#endif
