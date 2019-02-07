.section ".text.boot"
.global _start
_start:
    // Setup stack
    mov sp, #0x80000

    // TODO: zero out .bss here

    // Jump to kernel
    b kernel_main
1:
    b 1b
