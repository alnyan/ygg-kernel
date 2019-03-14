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
    if (signum) {
        kdebug("Terminating %d with signal %d\n", task_ctl(t)->pid, signum);
    } else {
        kdebug("Task exited with status %d\n", task_status(t));
    }

    sched_remove(t);
}

void task_busy(task_t *t) {
    task_ctl(t)->flags |= TASK_FLG_BUSY;
}

vfs_node_t *task_get_fd(task_t *t, int fd) {
    if (fd < 0 || fd >= 4) {
        return NULL;
    }

    return task_ctl(t)->fds[fd];
}
