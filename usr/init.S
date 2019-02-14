.section .text
.global _start
_start:
    movl 4(%esp), %edi

    movl $(0x0700 | 'A'), %edx
    xorl %ecx, %ecx
1:
    movl $10, %eax
    int $0x80

    incl %ecx
    andl $1, %ecx
    test %ecx, %ecx
    jz 2f

    // ecx is 1
    movw %dx, (%edi)

    jmp 3f
2:
    // ecx is 0
    movw $0, (%edi)
3:
    // Put this task into sleep for some time
    jmp 1b
