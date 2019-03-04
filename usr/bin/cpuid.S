.extern printf
.extern puts

.section .text
.global main
main:
    push %ebp
    mov %esp, %ebp

    // Allocate a 12-character buffer + zero terminator + padding
    sub $24, %esp

    push $cpuid_welcome_str
    call puts
    add $4, %esp

// VENDOR SECTION
    // Get CPU vendor
    xor %eax, %eax
    cpuid

    // Store vendor name on stack
    movl %ebx, (%esp)
    movl %edx, 4(%esp)
    movl %ecx, 8(%esp)
    movl $0,   12(%esp)
    movl %eax, lfuncstd

    // Print largest usable function
    push %eax
    push $cpuid_lfuncstd_fmt
    call printf
    add $8, %esp

    // Print CPU vendor
    push %esp
    push $cpuid_vendor_fmt
    call printf
    add $8, %esp

// FEATURE SECTION
    // Get CPU feature set
    mov $1, %eax
    cpuid
    // Store features
    movl %edx, (%esp)
    movl %ecx, 4(%esp)

    push %ecx
    push %edx
    push $cpuid_features_fmt
    call printf
    add $12, %esp

    push $cpuid_features_str
    call puts
    add $4, %esp

    // EDX feature list
    xor %ecx, %ecx
    mov $1, %ebx
.feature_loop_edx:
    cmp $19, %ecx
    je 1f

    movl (%esp), %edx
    test %edx, %ebx
    jz 1f

    // CPU has EDX feature %ecx available
    mov $cpuid_feature_edx, %eax
    mov %ecx, %edx
    shl $3, %edx
    addl %edx, %eax

    push %ecx
    push %ebx

    push %eax
    call puts
    add $4, %esp

    pop %ebx
    pop %ecx
1:

    inc %ecx
    shl %ebx
    cmp $32, %ecx
    jne .feature_loop_edx

    // ECX feature list
    xor %ecx, %ecx
    mov $1, %ebx
.feature_loop_ecx:
    cmp $19, %ecx
    je 1f

    movl 4(%esp), %edx
    test %edx, %ebx
    jz 1f

    // CPU has ECX feature %ecx available
    mov $cpuid_feature_ecx, %eax
    mov %ecx, %edx
    shl $4, %edx
    addl %edx, %eax

    push %ecx
    push %ebx

    push %eax
    call puts
    add $4, %esp

    pop %ebx
    pop %ecx
1:

    inc %ecx
    shl %ebx
    cmp $27, %ecx
    jne .feature_loop_ecx

// CPU cache and TLB info, if available
    cmpl $2, lfuncstd
    jl .notlb

    push %edx
    push %ebx
    push %eax
    push $cpuid_tlb_fmt
    call printf
    add $16, %esp

    jmp 1f
.notlb:
    push $cpuid_notlb_str
    call puts
    add $4, %esp
1:
// Extension functions
    mov $0x80000000, %eax
    cpuid

    mov %eax, lfuncext

    push %eax
    push $cpuid_lfuncext_fmt
    call printf
    add $8, %esp

    mov $0x80000000, %esi

    // Dump extension values
.funcext_loop:
    mov %esi, %eax
    cpuid

    push %esi

    push %edx
    push %ecx
    push %ebx
    push %eax
    push %esi
    push $cpuid_funcext_fmt
    call printf
    add $24, %esp

    pop %esi

    inc %esi
    cmp lfuncext, %esi
    jle .funcext_loop

    xor %eax, %eax
    push %eax
    call exit
    add $4, %esp

    mov %ebp, %esp
    pop %ebp

    ret

// EDX features
cpuid_feature_edx:
    .string "* fpu  "
    .string "* vme  "
    .string "* de   "
    .string "* pse  "
    .string "* tsc  "
    .string "* msr  "
    .string "* pae  "
    .string "* mce  "
    .string "* cx8  "
    .string "* apic "
    .string "* sep  "
    .string "* mtrr "
    .string "* pge  "
    .string "* mca  "
    .string "* cmov "
    .string "* pat  "
    .string "* pse36"
    .string "* psn  "
    .string "* clf  "
    .string "* RES  "
    .string "* RES  "
    .string "* dtes "
    .string "* acpi "
    .string "* mmx  "
    .string "* fxsr "
    .string "* sse  "
    .string "* sse2 "
    .string "* ss   "
    .string "* htt  "
    .string "* tm1  "
    .string "* ia64 "
    .string "* pbe  "

// ECX features
cpuid_feature_ecx:
    .string "* sse3         "
    .string "* pclmul       "
    .string "* dtes64       "
    .string "* monitor      "
    .string "* ds_cpl       "
    .string "* vmx          "
    .string "* smx          "
    .string "* est          "
    .string "* tm2          "
    .string "* ssse3        "
    .string "* cid          "
    .string "* RES          "
    .string "* fma          "
    .string "* cmpxchg16b   "
    .string "* etprd        "
    .string "* pdcm         "
    .string "* pcide        "
    .string "* dca          "
    .string "* sse4.1       "
    .string "* sse4.2       "
    .string "* x2-apic      "
    .string "* movbe        "
    .string "* popcnt       "
    .string "* RES          "
    .string "* aes          "
    .string "* xsave        "
    .string "* osxsave      "
    .string "* avx          "

cpuid_features_str:
    .string "Supported features:"
cpuid_welcome_str:
    .string "Welcome to Mark Poliakov's CPUID util"
cpuid_notlb_str:
    .string "CPUID reported no TLB/Cache features"

cpuid_features_fmt:
    .string "Feature registers: edx = 0x%08x, ecx = 0x%08x\n"
cpuid_vendor_fmt:
    .string "CPU vendor string: \"%s\"\n"
cpuid_value_fmt:
    .string "Value 0x%x\n"
cpuid_lfuncstd_fmt:
    .string "Largest standard function number: 0x%x\n"
cpuid_lfuncext_fmt:
    .string "Largest extension function number: 0x%x\n"
cpuid_tlb_fmt:
    .string "TLB Descriptors:\n\teax = 0x%08x\n\tebx = 0x%08x\n\tecx = 0x%08x\n\tedx = 0x%08x\n"
cpuid_funcext_fmt:
    .string "Extension function 0x%08x:\n\teax = 0x%08x\n\tebx = 0x%08x\n\tecx = 0x%08x\n\tedx = 0x%08x\n"

.section .data
lfuncstd:
    .long 0
lfuncext:
    .long 0
