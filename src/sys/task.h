#pragma once
#include <stdint.h>

typedef void task_t;
typedef void *(*task_entry_func)(void *);
typedef int pid_t;

// Platform-agnostic task control struct
typedef struct {
    // TODO: file descriptor table
    uint32_t sleep;
    pid_t pid;
} task_ctl_t;

#define TASK_FLG_RUNNING        (1 << 0)
#define TASK_FLG_STOP           (1 << 1)
#define TASK_FLG_WAIT           (1 << 2)
#define TASK_FLG_BUSY           (1 << 3)

task_ctl_t *task_ctl_create(void);
void task_ctl_free(task_ctl_t *ctl);

void task_busy(task_t *task);
void task_nobusy(task_t *task);

task_t *task_create(void);
int task_init(task_t *t, task_entry_func entry, void *arg, uint32_t flags);
void task_destroy(task_t *t);

// TODO: move this to sched.h?
void task_enable();
