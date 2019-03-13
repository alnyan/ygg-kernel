.section .text
.global x86_isr_14

x86_isr_14:
    cli
1:
    hlt
    jmp 1b
