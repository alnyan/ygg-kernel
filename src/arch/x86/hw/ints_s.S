.section .bss
.global x86_crash_esp
x86_crash_esp:
    .skip 4

.section .text
.extern x86_isr_handler
.extern x86_int_stack

x86_isr_generic:
    pushal

    // Stack state:
    // ss           <esp + 56>
    // esp          <esp + 52>
    // eflags       <esp + 48>
    // cs           <esp + 44>
    // error code   <esp + 40>
    // int num      <esp + 36>
    // eip          <esp + 32>
    // eax          <esp + 28>
    // ecx          <esp + 24>
    // edx          <esp + 20>
    // ebx          <esp + 16>
    // oesp         <esp + 12>
    // ebp          <esp + 08>
    // esi          <esp + 04>
    // edi          <esp + 00>
    movl %esp, %esi

    // Now select which stack to use (we may overwrite some important info if we use kernel stack
    // when exception happens in kernel mode (which means they both use the same stack)
    movl 44(%esi), %eax
    cmpl $0x08, %eax
    jne 1f

    movl %ebp, x86_crash_esp
    // cs is 0x08, use a dedicated kernel exception stack
    movl $x86_kernel_exc_stack_end, %esp
    jmp 2f
1:
    // cs is in userspace, use plain kernel stack
    movl $x86_int_stack, %esp

    jmp 2f
2:

    // x86_isr_handler(&task_ctx)
    pushl %esi
    call x86_isr_handler
    addl $4, %esp

    // No recoverable error support yet
3:
    hlt
    jmp 3b

.macro x86_isr_stub_nc n
.global x86_isr_\n
x86_isr_\n:
    cli
    pushl $0 // Compensate for missing errcode
    pushl $\n
    jmp x86_isr_generic
.endm

.macro x86_isr_stub n
.global x86_isr_\n
x86_isr_\n:
    cli
    pushl $\n
    jmp x86_isr_generic
.endm

x86_isr_stub_nc 0
x86_isr_stub_nc 1
x86_isr_stub_nc 2
x86_isr_stub_nc 3
x86_isr_stub_nc 4
x86_isr_stub_nc 5
x86_isr_stub_nc 6
x86_isr_stub_nc 7
x86_isr_stub    8
x86_isr_stub_nc 9
x86_isr_stub    10
x86_isr_stub    11
x86_isr_stub    12
x86_isr_stub    13
x86_isr_stub_nc 15
x86_isr_stub_nc 16
x86_isr_stub    17
x86_isr_stub_nc 18
x86_isr_stub_nc 19
x86_isr_stub_nc 20
x86_isr_stub_nc 21
x86_isr_stub_nc 22
x86_isr_stub_nc 23
x86_isr_stub_nc 24
x86_isr_stub_nc 25
x86_isr_stub_nc 26
x86_isr_stub_nc 27
x86_isr_stub_nc 28
x86_isr_stub_nc 29
x86_isr_stub    30
x86_isr_stub_nc 31

.section .bss
x86_kernel_exc_stack:
    .skip 1024
x86_kernel_exc_stack_end:
