#include "task.h"
#include "sys/task.h"
#include "sys/assert.h"
#include "sys/mem.h"
#include "sys/heap.h"
#include "arch/hw.h"
#include "sys/debug.h"
#include "sys/sched.h"
#include "sys/kinit.h"

#define ctx0(p)         ((x86_task_ctx_t *) (((x86_task_t *) (p))->kstack_esp))

int x86_task_init = 0;

task_t *task_create(void) {
    x86_task_t *t = (x86_task_t *) heap_alloc(sizeof(x86_task_t));
    assert(t);
    memset(t, 0, sizeof(x86_task_t));

    assert((t->ctl = task_ctl_create()));

    // Create primary kernel stack
    void *kstack = heap_alloc(4096);
    assert(kstack);
    memset(kstack, 0, 4096);

    t->kstack_esp = (uintptr_t) kstack + 4096 - 19 * 4;
    t->kstack_base = (uintptr_t) kstack;
    t->kstack_size = 4096;

    ctx0(t)->iret.eflags = 0x248;

    return t;
}

void task_init(void) {
    // kinit/idle task, which also performs userspace entry to init task
    task_t *kinit = task_create();

    ((x86_task_t *) kinit)->space = mm_kernel;
    ctx0(kinit)->cr3 = (uintptr_t) mm_kernel - KERNEL_VIRT_BASE;

    assert(x86_task_eip(kinit, 0x08, (uintptr_t) kinit_task) == 0);

    sched_add(kinit);
    sched_set_idle(kinit);
}

////

int x86_task_eip(x86_task_t *t, uint8_t cs, uintptr_t eip) {
    ctx0(t)->iret.cs = cs;
    ctx0(t)->iret.eip = eip;

    switch (cs) {
    case 0x08:
        ctx0(t)->seg.gs = 0x10;
        ctx0(t)->seg.fs = 0x10;
        ctx0(t)->seg.es = 0x10;
        ctx0(t)->seg.ds = 0x10;
        break;
    case 0x1B:
        ctx0(t)->seg.gs = 0x23;
        ctx0(t)->seg.fs = 0x23;
        ctx0(t)->seg.es = 0x23;
        ctx0(t)->seg.ds = 0x23;
        break;
    default:
        panic("Invalid code segment\n");
    }

    return 0;
}

int x86_task_space(x86_task_t *t, mm_space_t pd, uintptr_t pd_phys) {
    kdebug("Space\n");

    ctx0(t)->cr3 = pd_phys;
    t->space = pd;

    return 0;
}

int x86_task_user_alloc(x86_task_t *t, uintptr_t base, size_t npages) {
    if (task_ctl(t)->flags & TASK_FLG_KERNEL) {
        return -1;
    }

    for (int i = 0; i < npages; ++i) {
        uintptr_t page_phys = mm_alloc_physical_page(0);
        assert(page_phys != MM_NADDR);
        assert(mm_map_range_pages(task_space(t), base + i * 0x1000, &page_phys, 1, MM_FLG_US | MM_FLG_WR) == 0);
    }

    t->ustack_base = base;
    t->ustack_size = 0x1000 * npages;

    ctx0(t)->iret.ss = 0x23;
    ctx0(t)->iret.esp = t->ustack_base + t->ustack_size;

    return 0;
}

#undef ctx0
