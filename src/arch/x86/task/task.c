#include "task.h"
#include "arch/hw.h"
#include <stddef.h>
#include "../hw/hw.h"
#include "../multiboot.h"
#include "sys/panic.h"
#include "sys/task.h"
#include "sys/elf.h"
#include "sys/heap.h"
#include "sys/string.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "sys/attr.h"
#include "sys/mm.h"
#include "sys/mem.h"
#include "sys/time.h"

task_t *task_create(void) {
    struct x86_task *t = (struct x86_task *) heap_alloc(sizeof(struct x86_task));
    memset(t, 0, sizeof(struct x86_task *));
    t->ctl = task_ctl_create();
    t->next = NULL;

    return (task_t *) t;
}

void task_destroy(task_t *t) {
    mm_set(mm_kernel);
    struct x86_task *task = (struct x86_task *) t;

    // TODO: move to task_ctl_free()
    // Close file descriptors
    // for (int i = 0; i < 4; ++i) {
    //     if (task->ctl->fds[i]) {
    //         vfs_close(task->ctl->fds[i]);
    //     }
    // }

    // Free stacks
    heap_free((void *) task->esp0);

    // Free pagedir
    mm_destroy_space(task->pd);

    // Free data structures
    task_ctl_free(task->ctl);
    heap_free(task);
}

task_t *task_by_pid(int pid) {
    for (struct x86_task *t = x86_task_first; t; t = t->next) {
        if (t->ctl) {
            if (t->ctl->pid == pid) {
                return t;
            }
        } else if (pid == 0) {
            return t;
        }
    }

    return NULL;
}

// Idle task (kernel-space) stuff
static struct x86_task x86_task_idle;
static uint32_t x86_task_idle_stack[X86_TASK_TOTAL_STACK];

// Scheduling stuff
struct x86_task *x86_task_current = NULL;
struct x86_task *x86_task_first = NULL;
struct x86_task *x86_task_last = NULL;

int x86_last_pid = 0;

void task_enable(task_t *t) {
    kdebug("Adding task %p to sched\n", t);

    struct x86_task *task = (struct x86_task *) t;

    if (task->ctl) {
        // FIXME: move to platform-generic
        task->ctl->pid = ++x86_last_pid;
    }
    task->flag = 0;

    x86_task_last->next = t;
    x86_task_last = t;
}

// If set to 1, means we haven't entered multitasking yet
int x86_tasking_entry = 1;

// Assembly function which uses no stack and just loops with sti; hlt;
extern void x86_task_idle_func(void *arg);

void x86_task_dump_context(int level, struct x86_task *task) {
    if (task->ctl) {
        kprint(level, "--- General task info ---\n");
        kprint(level, "PID = %u\n", task->ctl->pid);
    }

    if (task->pd) {
        kprint(level, "--- Task address space ---\n");
        kprint(level, "Physical address: %p\n", task->pd[0]);
        mm_dump_map(level, task->pd);
    }

    if (task->esp0) {
        struct x86_task_context *ctx = (struct x86_task_context *) task->esp0;

        kprint(level, "--- Task context ---\n");
        kprint(level, "CS:EIP = %02x:%p\n", ctx->iret.cs, ctx->iret.eip);
        if (ctx->iret.cs != 0x08) {
            kprint(level, "SS:ESP = %02x:%p\n", ctx->iret.ss, ctx->iret.esp);
        }

        kprint(level, "--- Task registers ---\n");
        kprint(level, "eax = %p (%d)\n", ctx->gp.eax, ctx->gp.eax);
        kprint(level, "ecx = %p (%d)\n", ctx->gp.ecx, ctx->gp.ecx);
        kprint(level, "edx = %p (%d)\n", ctx->gp.edx, ctx->gp.edx);
        kprint(level, "ebx = %p (%d)\n", ctx->gp.ebx, ctx->gp.ebx);
    }
}

int x86_task_set_context(struct x86_task *task, uintptr_t entry, void *arg, uint32_t flags) {
    struct x86_task_context *ctx;
    struct x86_task_context *sigctx = NULL;
    uintptr_t cr3;

    task->sigesp = 0;
    task->sigeip = 0;

    if (!task->esp0) {
        // Create task's kernel stack
        uintptr_t ebp0 = (uintptr_t) heap_alloc(19 * 4) + 19 * 4;
        assert(ebp0);
        ctx = (struct x86_task_context *) (ebp0 - 19 * 4);
        memset(ctx, 0, sizeof(struct x86_task_context));

        task->ebp0 = ebp0;
        task->esp0 = ebp0 - 19 * 4;
    } else {
        ctx = (struct x86_task_context *) task->esp0;
    }

    if (!(flags & (X86_TASK_NOSIGCTX | X86_TASK_IDLE))) {
        sigctx = (struct x86_task_context *) heap_alloc(19 * 4);
        assert(sigctx);
        task->ctl->sigctx = sigctx;
    }

    if (!task->pd) {
        task->pd = mm_create_space(&cr3);
        mm_space_clone(task->pd, mm_kernel, MM_FLG_CLONE_KERNEL);

        ctx->cr3 = cr3;
    } else {
        assert((cr3 = mm_translate(mm_kernel, (uintptr_t) task->pd, NULL)) != MM_NADDR);

        if (!ctx->cr3) {
            ctx->cr3 = cr3;
        }
    }

    if (!(flags & X86_TASK_NOGP)) {
        memset(&ctx->gp, 0, 8 * 4);
    }

    if (!(flags & X86_TASK_NOENT)) {
        ctx->iret.cs = (flags & X86_TASK_IDLE) ? 0x08 : 0x1B;
        ctx->iret.eip = entry;
    }

    ctx->iret.eflags = 0x248;

    if (!(flags & (X86_TASK_NOESP3 | X86_TASK_IDLE))) {
        uint32_t *esp3;

        if (!task->esp3_bottom) {
            // Allocate user stack
            task->esp3_bottom = 0x80000000;
            if (!task->esp3_size) {
                task->esp3_size = 8;
            }
            assert(mm_map_range(task->pd, task->esp3_bottom, task->esp3_size, MM_FLG_US | MM_FLG_WR) == 0);
        }

        kdebug("TASK ESP3_BOTTOM = %p\n", task->esp3_bottom);
        esp3 = (uint32_t *) (task->esp3_bottom + task->esp3_size * 0x1000);

        uint32_t user_stack[] = {
            (uint32_t) arg,
            0x12345678,
        };
        esp3 -= sizeof(user_stack);
        mm_memcpy_kernel_to_user(task->pd, esp3, user_stack, sizeof(user_stack));

        ctx->iret.ss = 0x23;
        ctx->iret.esp = (uintptr_t) esp3;
    }

    if (flags & X86_TASK_IDLE) {
        ctx->segs.ds = 0x10;
        ctx->segs.es = 0x10;
        ctx->segs.fs = 0x10;
        ctx->segs.gs = 0x10;
    } else {
        ctx->segs.ds = 0x23;
        ctx->segs.es = 0x23;
        ctx->segs.fs = 0x23;
        ctx->segs.gs = 0x23;
    }

    return 0;
}

void task_set_kernel(task_t *t, task_entry_func entry, void *arg, uint32_t flags) {
    // Create a kernel stack
    uintptr_t esp0 = (uintptr_t) heap_alloc(2048);
    struct x86_task *task = (struct x86_task *) t;

    task->next = NULL;
    task->ctl = NULL;
    task->flag = 0;

    task->esp0 = esp0 + 2048 - 19 * 4;
    task->ebp0 = esp0 + 2048;
    task->esp3_bottom = 0;
    task->esp3_size = 0;
    task->pd = mm_kernel;

    assert(x86_task_set_context(task, (uintptr_t) entry, arg, X86_TASK_IDLE | X86_TASK_NOESP3) == 0);
}

int x86_task_enter_signal(struct x86_task *task) {
    assert(task && task->ctl);
    // If userspace libc defined any signal entry point
    if (task->ctl->sigctx && task->sigeip) {
        // If signalled task has a sleep/wait pending, abort it and set -1 error code
        if (task->flag & TASK_FLG_WAIT) {
            ((struct x86_task_context *) task->esp0)->gp.eax = -1;
            task->flag &= ~TASK_FLG_WAIT;
        }

        struct x86_task_context *sigctx = (struct x86_task_context *) task->ctl->sigctx;

        // Setup signal handler context
        memset(&sigctx->gp, 0, 8 * 4);
        sigctx->gp.edx = task->ctl->pending_signal;

        sigctx->segs.gs = 0x23;
        sigctx->segs.fs = 0x23;
        sigctx->segs.es = 0x23;
        sigctx->segs.ds = 0x23;


        sigctx->iret.eflags = 0x248;
        sigctx->iret.cs = 0x1B;
        sigctx->iret.eip = task->sigeip;
        sigctx->iret.esp = task->sigesp;
        sigctx->iret.ss = 0x23;

        sigctx->cr3 = ((struct x86_task_context *) (task->ebp0 - 19 * 4))->cr3;

        task->esp0 = (uintptr_t) task->ctl->sigctx;

        return 0;
    } else {
        return -1;
    }
}

int x86_task_exit_signal(struct x86_task *task) {
    assert(task && task->ctl && task->ctl->sigctx);
    task->esp0 = task->ebp0 - 19 * 4;
    return 0;
}

void x86_task_init(void) {
    kdebug("Initializing multitasking\n");

    x86_task_idle.next = NULL;
    x86_task_idle.ctl = NULL;
    x86_task_idle.flag = 0;

    x86_task_idle.ebp0 = (uintptr_t) x86_task_idle_stack + 19 * 4;
    x86_task_idle.esp0 = (uintptr_t) x86_task_idle_stack;
    x86_task_idle.esp3_size = 0;
    x86_task_idle.esp3_bottom = 0;
    x86_task_idle.pd = mm_kernel;

    x86_task_set_context(&x86_task_idle, (uintptr_t) x86_task_idle_func, NULL, X86_TASK_IDLE | X86_TASK_NOESP3);

    x86_task_current = &x86_task_idle;
    x86_task_first = &x86_task_idle;
    x86_task_last = &x86_task_idle;
}

void x86_task_switch(x86_irq_regs_t *regs) {
    mm_set(mm_kernel);

    if (x86_tasking_entry) {
        x86_tasking_entry = 0;
        return;
    }

    struct x86_task *tp = NULL;
    // Update tasks' flags and ctl
    for (struct x86_task *t = x86_task_first; t; t = t->next) {
        if (t == &x86_task_idle) {
            tp = t;
            continue;
        }

        // Task stopped
        if (t->flag & TASK_FLG_STOP) {
            // Remove task from sched here
            assert(tp);

            tp->next = t->next;
            if (t == x86_task_last) {
                x86_task_last = tp;
            }

            int res;
            if (t->ctl->pending_signal) {
                res = 1000 + t->ctl->pending_signal;
            } else {
                res = *((uint32_t *) (t->ebp0 - 8 * 4));
            }

            if (res < 1000) {
                kdebug("Task %d exited with status %d\n", t->ctl->pid, res);
            } else {
                kwarn("Task %d exited because of an error:\n", t->ctl->pid);
                switch (t->ctl->pending_signal) {
                case SIGABRT:
                    kwarn("  abort() was called\n");
                    break;
                case SIGSEGV:
                    kwarn("  Address space violation\n");
                    break;
                case SIGQUIT:
                    kwarn("  Quit signal\n");
                    break;

                case SIGUSR1:
                    kwarn("  User signal 1\n");
                    break;
                case SIGUSR2:
                    kwarn("  User signal 2\n");
                    break;

                default:
                    kwarn("  Unspecified\n");
                    break;
                }
            }

            // Attempted to kill root process
            if (t->ctl->pid == 1) {
                panic_irq("Attempted to kill init!\n", regs);
            }

            task_destroy(t);

            continue;
        }
        // Decrease sleep counters
        if (t->flag & TASK_FLG_WAIT) {
            switch (t->wait_type) {
            case TASK_WAIT_SLEEP:
                if (systime >= t->ctl->sleep_deadline) {
                    t->flag &= ~TASK_FLG_WAIT;
                }
                break;
            case TASK_WAIT_PID:
                {
                    // TODO: a better way to implement this
                    // Maybe signal from quitting task directly into this
                    uint32_t pid = (uint32_t) t->ctl->sleep_deadline;
                    task_t *t2 = task_by_pid(pid);
                    if (!t2) {
                        t->ctl->sleep_deadline = 0;
                        t->flag &= ~TASK_FLG_WAIT;
                    }
                }
            }
        }
        tp = t;
    }

    struct x86_task *from = x86_task_current;

    x86_task_current->flag &= ~TASK_FLG_RUNNING;

    while (1) {
        if (!(x86_task_current = x86_task_current->next)) {
            x86_task_current = x86_task_first;
        }

        if ((x86_task_current->flag & TASK_FLG_WAIT) || (x86_task_current->flag & TASK_FLG_BUSY)) {
            // If a loop was done and we couldn't find any task, just use the previous one
            if (x86_task_current == from) {
                break;
            }
            continue;
        }

        break;
    }

    if (x86_task_current == from &&
            ((x86_task_current->flag & TASK_FLG_WAIT) ||
             (x86_task_current->flag & TASK_FLG_BUSY))) {
        // If we couldn't find any non-waiting task
        x86_task_current = &x86_task_idle;
        x86_task_current->flag |= TASK_FLG_RUNNING;
        return;
    }

#ifdef ENABLE_SCHED_TRACE
    int from_pid = -1;
    int to_pid = -1;

    if (from && from->ctl) {
        from_pid = from->ctl->pid;
    }
    if (x86_task_current && x86_task_current->ctl) {
        to_pid = x86_task_current->ctl->pid;
    }

    if (from_pid != to_pid)
    kdebug("%d -> %d\n", from_pid, to_pid);
#endif

    x86_task_current->flag |= TASK_FLG_RUNNING;
}
