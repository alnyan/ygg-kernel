#include "task.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "sys/heap.h"
#include "sys/sched.h"

task_ctl_t *task_ctl_create(void) {
    task_ctl_t *ctl = (task_ctl_t *) heap_alloc(sizeof(task_ctl_t));
    assert(ctl);

    memset(ctl, 0, sizeof(task_ctl_t));

    return ctl;
}

void task_terminate(task_t *t, int signum) {
    task_ctl(t)->flags |= TASK_FLG_STOP;
    // TODO: push signal to signal queue
    kdebug("Terminating %d with signal %d\n", task_ctl(t)->pid, signum);

    sched_remove(t);
}
