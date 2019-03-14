#include "sched.h"
#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/mem.h"
#include "sys/heap.h"

struct sched_list {
    task_t *begin;
    task_t *end;
};

task_t *sched_current = NULL;

static int sched_last_pid = 0;
static task_t *sched_idle = NULL;
static struct sched_list sched_ready;
static struct sched_list sched_waiting;

static void sched_list_add(int l, task_t *t) {
    struct sched_list *dst = &sched_ready;
    if (l == 1) {
        dst = &sched_waiting;
    }

    task_next(t) = NULL;
    task_prev(t) = dst->end;

    if (dst->end) {
        task_next(dst->end) = t;
    } else {
        dst->begin = t;
    }

    dst->end = t;
}

static void sched_list_remove(int l, task_t *t, int c) {
    struct sched_list *src = &sched_ready;
    if (l == 1) {
        src = &sched_waiting;
    }

    // Remove task from queue
    task_t *prev = task_prev(t);
    task_t *next = task_next(t);

    if (t == src->begin) {
        if (next) {
            task_prev(next) = NULL;
        } else {
            src->end = NULL;
        }

        src->begin = next;
    } else {
        if (next) {
            task_prev(next) = prev;
        } else {
            src->end = prev;
        }

        task_next(prev) = next;
    }

    if (c && t == sched_current) {
        sched_current = NULL;
    }
}

void sched_init(void) {
    memset(&sched_ready, 0, sizeof(sched_ready));
    memset(&sched_waiting, 0, sizeof(sched_waiting));
}

int sched_add(task_t *t) {
    kdebug("%d\n", sched_last_pid);
    task_ctl(t)->pid = sched_last_pid;

    sched_list_add(0, t);

    return sched_last_pid++;
}

void sched_busy(task_t *t, int b) {
    sched_list_remove(!b, t, 0);
    sched_list_add(b, t);
}

void sched_remove(task_t *t) {
    if (!(task_ctl(t)->flags & (TASK_FLG_BUSY | TASK_FLG_WAIT))) {
        sched_list_remove(0, t, 1);

        // TODO: destroy the task
        kdebug("Task %d finished\n", task_ctl(t)->pid);
    } else {
        sched_list_remove(1, t, 1);
    }
}

void sched_set_idle(task_t *t) {
    assert(!sched_idle);
    sched_idle = t;
}

void sched(void) {
    if (!sched_current) {
        sched_current = sched_idle;
        return;
    }

    task_t *from = sched_current;
    task_t *to = task_next(from);

    if (!to) {
        to = sched_ready.begin;
    }

    assert(!(task_ctl(to)->flags & (TASK_FLG_BUSY | TASK_FLG_WAIT)));

    sched_current = to;
}
