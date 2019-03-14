#pragma once
#include "sys/task.h"

extern task_t *sched_current;

void sched_init(void);

void sched_set_idle(task_t *t);
int sched_add(task_t *t);
task_t *sched_find(pid_t pid);

void sched_busy(task_t *t, int b);

void sched_remove(task_t *t);

void sched(void);
