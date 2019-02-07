.section ".text.boot"
.global _start
_start:
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, 2f
1:
    wfe
    b 1b
2:
    // Setup stack
    mov sp, #0x80000

    // TODO: zero out .bss here

    // Jump to kernel
    b kernel_main
3:
    b 3b
