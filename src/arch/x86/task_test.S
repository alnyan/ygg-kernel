.section .text
.global task0
.global end_task0
.global task0_var
.global end_task0_var
task0:
    // Setup memory access addr
    // WARNING: the code assumes kernel allocates page #769 for it
    movl $(task0_var - task0 + 0xC0400000), %esi
    movl (%esi), %edi
    movl $0, %ecx

1:

    incl %ecx
    andl $0x1, %ecx
    test %ecx, %ecx

    jz 2f

    // ecx = 1
    movl 4(%esi), %eax
    movl 8(%esi), %edx
    orl %edx, %eax
    movl %eax, (%edi)

    jmp 1b
2:
    // ecx = 0
    movl $0, (%edi)

    jmp 1b
end_task0:
task0_var:
    .long 0xB8000
    .long 'A'
    .long 0x0700
end_task0_var:
