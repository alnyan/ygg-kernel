#include "task.h"
#include "sys/mem.h"
#include "sys/assert.h"
#include "sys/heap.h"

task_ctl_t *task_ctl_create(void) {
    task_ctl_t *ctl = (task_ctl_t *) heap_alloc(sizeof(task_ctl_t));
    assert(ctl);

    memset(ctl, 0, sizeof(task_ctl_t));

    return ctl;
}
