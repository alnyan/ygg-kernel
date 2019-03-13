// A bunch of asm macros so we don't need to change the code in all places

.set X86_TASK_ESP0,     0x00
.set X86_TSS_ESP0,      0x04

.extern mm_kernel
.extern sched_current

// Assumes %esp is correctly placed in task's kstack
.macro x86_int_entry
    pushal

    mov %cr3, %eax
    push %eax

    mov %gs, %eax
    push %eax
    mov %fs, %eax
    push %eax
    mov %es, %eax
    push %eax
    mov %ds, %eax
    push %eax

    mov $0x10, %eax
    mov %eax, %gs
    mov %eax, %fs
    mov %eax, %es
    mov %eax, %ds

    // Switch to kernel PD
    mov mm_kernel, %eax
    subl $0xC0000000, %eax
    mov %eax, %cr3

    movl sched_current, %edi
    test %edi, %edi
    jz 1f

    // Store esp0
    movl %esp, X86_TASK_ESP0(%edi)

1:  // No task
.endm

// Assumes %esp is a pointer to new task's kstack
.macro x86_irq_master_exit
    // Restore the context
    pop %eax
    mov %eax, %ds
    pop %eax
    mov %eax, %es
    pop %eax
    mov %eax, %fs
    pop %eax
    mov %eax, %gs

    pop %eax
    mov %eax, %cr3

    // While we still can mess with gp registers, send EOI
    mov $0x20, %dx
    mov $0x20, %al
    outb %al, %dx

    popal
.endm

.macro x86_isr_exit
    // Restore the context
    pop %eax
    mov %eax, %ds
    pop %eax
    mov %eax, %es
    pop %eax
    mov %eax, %fs
    pop %eax
    mov %eax, %gs

    pop %eax
    mov %eax, %cr3

    popal
.endm
