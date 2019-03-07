.section .text
.global spin_lock
.global spin_unlock

spin_lock:
    movl 4(%esp), %edi
    movl $1, %ecx
1:
    xorl %eax, %eax
    lock cmpxchgl %ecx, (%edi)
    jnz 1b
    ret

spin_unlock:
    movl 4(%esp), %edi
    movl $0, (%edi)
    ret
