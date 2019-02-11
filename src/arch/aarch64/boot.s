/*
 * After passing the firmware stage, we're running at
 *  EL2 in multi-core mode.
 */

.section ".text.boot"
.global _start
_start:
    // Park all cores except 0th
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, 2f
1:
    wfe
    b 1b
2:
    // Enable EL1 access to timers
    mrs x0, cnthctl_el2
    orr x0, x0, #0b11
    msr cnthctl_el2, x0
    msr cntvoff_el2, xzr

    mov	x0, #(1 << 31)						// 64bit EL1
    msr hcr_el2, x0

    mov	x0, #3 << 20
    msr cpacr_el1, x0

    // Setup sctlr_el1
    // RES1 bits 29, 28, 23, 22, 20, 11
    // RES0 bits 31, 30, 27, 21, 17, 13, 10
    mov x0, #0x0800
    movk x0, #0x30D0, lsl #16
    orr x0, x0, #(1 << 2)       // Data caching
    orr x0, x0, #(1 << 12)      // Instruction caching
    msr sctlr_el1, x0

    // Exit to EL1_SP1 mode
    mov x0, #0x03C5
    msr spsr_el2, x0
    adr x0, .el1_sp1_exit
    msr elr_el2, x0
    eret

.el1_sp1_exit:
    // Setup stack
    mov sp, #0x80000

    // Setup EL1 interrupt vector
    ldr x0, =aarch64_exc_vectors
    msr vbar_el1, x0

    // TODO: zero out .bss here

    // Jump to kernel
    b kernel_main
3:
    wfe
    b 3b

.balign 0x4
exc_hang:
1:
    b 1b

.balign 0x4
.extern aarch64_irq_handler
exc_irq_handler_stub:
	stp	x29, x30, [sp, #-16]!
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	stp	x17, x18, [sp, #-16]!
	stp	x15, x16, [sp, #-16]!
	stp	x13, x14, [sp, #-16]!
	stp	x11, x12, [sp, #-16]!
	stp	x9, x10, [sp, #-16]!
	stp	x7, x8, [sp, #-16]!
	stp	x5, x6, [sp, #-16]!
	stp	x3, x4, [sp, #-16]!
	stp	x1, x2, [sp, #-16]!
	str	x0, [sp, #-16]!

    bl aarch64_irq_handler

	ldr x0, [sp], #16
	ldp x1, x2, [sp], #16
	ldp x3, x4, [sp], #16
	ldp x5, x6, [sp], #16
	ldp x7, x8, [sp], #16
	ldp x9, x10, [sp], #16
	ldp x11, x12, [sp], #16
	ldp x13, x14, [sp], #16
	ldp x15, x16, [sp], #16
	ldp x17, x18, [sp], #16
	ldp x19, x20, [sp], #16
	ldp x21, x22, [sp], #16
	ldp x23, x24, [sp], #16
	ldp x25, x26, [sp], #16
	ldp x27, x28, [sp], #16
	ldp x29, x30, [sp], #16

	eret

.macro vector handler
.balign 0x80
    b \handler
.endm

.section .data
.balign 0x800
.global aarch64_exc_vectors
aarch64_exc_vectors:
    vector exc_hang                 // Sync
    vector exc_hang                 // IRQ
    vector exc_hang                 // FIQ
    vector exc_hang                 // SError

    vector exc_hang                 // Sync
    vector exc_irq_handler_stub     // IRQ
    vector exc_hang                 // FIQ
    vector exc_hang                 // SError

    vector exc_hang                 // Sync
    vector exc_hang                 // IRQ
    vector exc_hang                 // FIQ
    vector exc_hang                 // SError

    vector exc_hang                 // Sync
    vector exc_hang                 // IRQ
    vector exc_hang                 // FIQ
    vector exc_hang                 // SError
