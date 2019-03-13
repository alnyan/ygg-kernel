.section .text

.set TSS_ESP0,              0x04

.set X86_TASK_STRUCT_ESP0,  0x00
.set X86_TASK_STACK,        18

.extern x86_int_stack

.macro irq_handler n
.extern x86_irq_handler_\n
.global x86_irq_\n
x86_irq_\n:
    cli
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
    iret
#endif
