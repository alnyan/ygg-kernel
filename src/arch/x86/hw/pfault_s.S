.section .text
.global x86_isr_14

x86_isr_14:
    cli
    add $4, %esp

    pushal

    mov %cr3, %eax
    push %eax

    pushl $0
    pushl $0
    pushl $0
    pushl $0

    push %esp
    call x86_page_fault
    add $4, %esp

    pop %eax
    mov %eax, %cr3

    pop %eax
    pop %eax
    pop %eax
    pop %eax

    popal
1:
    hlt
    jmp 1b
