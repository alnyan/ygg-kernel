.set KERNEL_VIRT_BASE, 0xC0000000
.set KERNEL_VIRT_PAGE, (KERNEL_VIRT_BASE >> 22)

.set ALIGN,    1 << 0
.set MEMINFO,  1 << 1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    mov $(__preserve_boot_ebx - KERNEL_VIRT_BASE), %edx
    mov %ebx, (%edx)

    // Clear out boot page directory
    movl $(boot_page_directory - KERNEL_VIRT_BASE), %edx
    movl $0x1000, %ecx
    xorl %eax, %eax
    rep stosl

    // Set zero-destination mapping from 0xC0000000 and 0
    movl $0x00000083, (boot_page_directory - KERNEL_VIRT_BASE + KERNEL_VIRT_PAGE * 4)
    movl $0x00000083, (boot_page_directory - KERNEL_VIRT_BASE)

    // Set paging directory
    movl $(boot_page_directory - KERNEL_VIRT_BASE), %eax
    movl %eax, %cr3

    // Enable PS (4 MiB pages)
    movl %cr4, %eax
    orl $0x10, %eax
    movl %eax, %cr4

    // Enable paging!
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    leal _start_high, %eax
    jmp *%eax

_start_high:

    // Unmap lower mapping
    movl $0, boot_page_directory

    // Setup stack
    movl $stack_top, %esp

    call kernel_main

    cli
1:	hlt
	jmp 1b

.size _start, . - _start

.section .data
.balign 0x4
__preserve_boot_ebx:
    .long 0x00000000

.section .bss
.balign 0x1000
boot_page_directory:
    .skip 0x1000

