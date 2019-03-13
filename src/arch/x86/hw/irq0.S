.section .text
.global x86_irq_0

.set X86_TASK_ESP0,     0x00
.set X86_TSS_ESP0,      0x04

.extern mm_kernel
.extern sched_current
.extern x86_task_init
.extern x86_timer_func
.extern x86_tss

x86_irq_0:
    // %ss
    // %esp
    // %eflags
    // %cs
    // %eip
    cli
    pushal
    // %eax
    // %ecx
    // %edx
    // %ebx
    // %esp
    // %ebp
    // %esi
    // %edi
    mov %cr3, %eax
    push %eax
    // cr3
    mov %gs, %eax
    push %eax
    mov %fs, %eax
    push %eax
    mov %es, %eax
    push %eax
    mov %ds, %eax
    push %eax
    // %gs
    // %fs
    // %es
    // %ds

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
    // Call timer handler (if set)
    movl x86_timer_func, %eax
    test %eax, %eax
    jz 1f
    call *%eax

1:  // No timer func
    call sched

    // New task is in sched_current
    mov sched_current, %esi
    test %esi, %esi
    jz 1f
    mov X86_TASK_ESP0(%esi), %esp

    // Set TSS entry to %esp + 18 * 4
    mov %esp, %ecx
    add $(18 * 4), %ecx
    mov %ecx, (X86_TSS_ESP0 + x86_tss)

1:  // Sched set task to null

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

    iret
