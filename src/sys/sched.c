#include "sched.h"
#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/mem.h"

struct sched_list {
    task_t *begin;
    task_t *end;
};

task_t *sched_current = NULL;

static int sched_last_pid = 0;
static task_t *sched_idle = NULL;
static struct sched_list sched_ready;
static struct sched_list sched_waiting;

void sched_init(void) {
    memset(&sched_ready, 0, sizeof(sched_ready));
    memset(&sched_waiting, 0, sizeof(sched_waiting));
}

int sched_add(task_t *t) {
    kdebug("%d\n", sched_last_pid);
    task_ctl(t)->pid = sched_last_pid;

    task_next(t) = NULL;

    if (sched_ready.end) {
        task_next(sched_ready.end) = t;
    } else {
        sched_ready.begin = t;
    }

    sched_ready.end = t;

    return sched_last_pid++;
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

    sched_current = to;
}
