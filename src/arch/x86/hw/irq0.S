.section .text
.global x86_irq_0
x86_irq_0:
    cli
    pushal

    mov $0x20, %dx
    mov $0x20, %al
    outb %al, %dx

    popal
    iret
