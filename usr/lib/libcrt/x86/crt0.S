.section .text
.global _start
_start:
    mov $0, %ebp
    push %ebp
    push %ebp
    mov %esp, %ebp

    // INIT LIBC HERE
    call __libc_signal_init

    call _init

    pushl $0
    // pushl 8(%esp)
    call main
    addl $4, %esp

    // sys_exit(%eax)
    mov %eax, %ebx
    mov $1, %eax
    int $0x80
1:
    jmp 1b

