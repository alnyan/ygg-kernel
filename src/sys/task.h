#pragma once
#include <stdint.h>
#include "sys/vfs.h"
#include "sys/attr.h"

typedef void task_t;
typedef void *(*task_entry_func)(void *);
typedef int pid_t;

// Platform-agnostic task control struct
typedef struct {
    vfs_file_t *fds[4];
    uint32_t sleep;
    pid_t pid;
} task_ctl_t;

#define TASK_FLG_RUNNING        (1 << 0)
#define TASK_FLG_STOP           (1 << 1)
#define TASK_FLG_WAIT           (1 << 2)
#define TASK_FLG_BUSY           (1 << 3)

task_t *task_fork(task_t *src);
task_t *task_fexecve(const char *path, const char **argp, const char **envp);
int task_execve(task_t *dst, const char *path, const char **argp, const char **envp);

task_ctl_t *task_ctl_create(void);
void task_ctl_free(task_ctl_t *ctl);

void task_busy(task_t *task);
void task_nobusy(task_t *task);

task_t *task_create(void);
int task_init(task_t *t, task_entry_func entry, void *arg, uint32_t flags);
void task_destroy(task_t *t);

void task_copy_to_user(task_t *t, userspace void *dst, const void *src, size_t siz);
void task_copy_from_user(task_t *t, void *dst, const userspace void *src, size_t siz);

// TODO: move this to sched.h?
void task_enable();
