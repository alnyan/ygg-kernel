.section .text

// We shouldn't use any kernel stack here
// As we're in Ring 0 and there's no user stack for us
.global x86_task_idle_func
x86_task_idle_func:
1:
    sti
    hlt
    jmp 1b
