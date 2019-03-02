#include "task.h"
#include "sys/heap.h"
#include "sys/mem.h"
#include "sys/debug.h"

task_ctl_t *task_ctl_create(void) {
    task_ctl_t *ctl = (task_ctl_t *) heap_alloc(sizeof(task_ctl_t));
    memset(ctl, 0, sizeof(task_ctl_t));
    kdebug("TASK CTL CREATE\n");
    return ctl;
}

void task_ctl_free(task_ctl_t *ctl) {
    heap_free(ctl);
}

void task_set_sleep(task_t *t, const struct timespec *ts) {
    uint64_t delta = ts->tv_sec * SYSTICK_DES_RES + ts->tv_nsec * (1000000 / SYSTICK_DES_RES);
    ((struct x86_task *) t)->ctl->sleep_deadline = delta + systime;
}

void task_busy(void *task) {
    ((struct x86_task *) task)->flag |= TASK_FLG_BUSY;
}

void task_nobusy(void *task) {
    ((struct x86_task *) task)->flag &= ~TASK_FLG_BUSY;
}
