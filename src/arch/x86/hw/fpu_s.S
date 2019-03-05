.section .text
.global x86_sse_enable
x86_sse_enable:
    mov %cr0, %eax
    andl $~(1 << 2), %eax
    orl $(1 << 1), %eax
    mov %eax, %cr0

    mov %cr4, %eax
    orl $((1 << 9) | (1 << 10)), %eax
    mov %eax, %cr4

    ret
