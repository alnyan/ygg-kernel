#pragma once
#include "../regs.h"
#include <stddef.h>
#include "sys/mm.h"

typedef struct x86_task x86_task_t;
typedef struct x86_task_ctx x86_task_ctx_t;
typedef struct task_ctl task_ctl_t;

#define task_space(t)           (((x86_task_t *) (t))->space)
#define task_ctl(t)             (((x86_task_t *) (t))->ctl)
#define task_next(t)            (((x86_task_t *) (t))->next)
#define task_prev(t)            (((x86_task_t *) (t))->prev)

struct x86_task_ctx {
    x86_seg_regs_t seg;
    uint32_t cr3;
    x86_gp_regs_t gp;
    x86_iret_regs_t iret;
};

struct x86_task {
    uintptr_t kstack_esp;       // == ctx
    uintptr_t kstack_base;
    uintptr_t kstack_size;

    uintptr_t kstack_sig_esp;   // == sigctx
    uintptr_t kstack_sig_base;
    uintptr_t kstack_sig_size;

    uintptr_t ustack_base;      // == esp3
    uintptr_t ustack_size;

    mm_space_t space;

    task_ctl_t *ctl;

    x86_task_t *prev, *next;
};

int x86_task_space(x86_task_t *t, mm_space_t pd, uintptr_t pd_phys);
int x86_task_user_alloc(x86_task_t *t, uintptr_t base, size_t npages);
int x86_task_eip(x86_task_t *t, uint8_t cs, uintptr_t eip);
