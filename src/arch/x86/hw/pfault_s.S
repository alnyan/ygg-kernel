.section .text

.set TSS_ESP0,              0x04
.set X86_TASK_STACK,        18

.set X86_TASK_STRUCT_ESP0,  0x00

.extern x86_task_current
.extern x86_task_first
.extern x86_task_switch
.extern x86_timer_func
.extern x86_int_stack

.extern x86_tss

.global x86_isr_14
x86_isr_14:
    // Stack here:
    cli
    // TODO: store this error code somehow
    add $4, %esp

    pushal

#ifdef ENABLE_TASK
    // Stack state:
    // <esp + 68> ss
    // <esp + 64> esp
    // <esp + 60> eflags
    // <esp + 56> cs
    // <esp + 52> eip
    // <esp + 48> eax
    // <esp + 44> ecx
    // <esp + 40> edx
    // <esp + 36> ebx
    // <esp + 32> oesp
    // <esp + 28> ebp
    // <esp + 24> esi
    // <esp + 20> edi
    // <esp + 16> cr3
    // <esp + 12> gs
    // <esp + 08> fs
    // <esp + 04> es
    // <esp + 00> ds

    movl %cr3, %eax
    pushl %eax

    pushl %gs
    pushl %fs
    pushl %es
    pushl %ds

    // The stack pointer now should be equal to FROM task's esp0
    // Setup kernel segments

    movl $0x10, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs
    movl %eax, %gs

    // Setup kernel interrupt stack pointer and store FROM stack in %esi
    movl %esp, %esi
    movl $x86_int_stack, %esp

    pushl %esi
    call x86_page_fault
    addl $4, %esp

    // %esi = TO task ptr
    movl x86_task_current, %esi

    // Load TO task's esp0 to %eax and %edx
    movl X86_TASK_STRUCT_ESP0(%esi), %eax
    movl %eax, %edx

    // Write TSS entry, eax + 19 * 4
    addl $(X86_TASK_STACK * 4), %edx
    movl %edx, x86_tss + TSS_ESP0

    // Now, esp should be TO task's esp0
    movl %eax, %esp

    // Remove pointer from stack
    popl %eax
    movl %eax, %ds
    popl %eax
    movl %eax, %es
    popl %eax
    movl %eax, %fs
    popl %eax
    movl %eax, %gs

    popl %eax
    movl %eax, %cr3
#else
.halt:
    hlt
    jmp .halt
#endif

    popal

    iretl
