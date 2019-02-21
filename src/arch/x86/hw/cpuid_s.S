.section .text
.global x86_cpuid_support
x86_cpuid_support:
    pushfl
    pushfl
    xorl $(1 << 21), (%esp)
    popfl
    pushfl
    popl %eax
    xorl (%esp), %eax
    popfl
    andl $(1 << 21), %eax
    retl
