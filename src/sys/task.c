#include "task.h"
#include "sys/heap.h"
#include "sys/mem.h"

task_ctl_t *task_ctl_create(void) {
    task_ctl_t *ctl = (task_ctl_t *) heap_alloc(sizeof(task_ctl_t));
    memset(ctl, 0, sizeof(task_ctl_t));
    return ctl;
}

void task_ctl_free(task_ctl_t *ctl) {
    heap_free(ctl);
}
