#pragma once
#include <stdint.h>
#include "sys/vfs.h"
#include "sys/attr.h"
#include "sys/time.h"
#include <uapi/signum.h>

#define TASK_EXIT_CODE(n)       (n & 0xFF)
// Exit codes 1000 >= are allocated for error statuses
#define TASK_EXIT_SEGV          1000

typedef void task_t;
typedef void *(*task_entry_func)(void *);
typedef int pid_t;

// Platform-agnostic task control struct
typedef struct {
    vfs_file_t *fds[4];
    uint64_t sleep_deadline;
    pid_t pid;
    // TODO: proper signal queue
    int pending_signal;
    void *sigctx;
} task_ctl_t;

#define TASK_FLG_RUNNING        (1 << 0)
#define TASK_FLG_STOP           (1 << 1)
#define TASK_FLG_WAIT           (1 << 2)
#define TASK_FLG_BUSY           (1 << 3)
#define TASK_WAIT_SLEEP         1
#define TASK_WAIT_PID           2

#if defined(ARCH_X86)
#include "arch/x86/task/task.h"
#define task_space(t)   ((mm_space_t) (((struct x86_task *) t)->pd))
#define task_ctl(t)     (((struct x86_task *) t)->ctl)
#endif

task_t *task_fork(task_t *src);
task_t *task_fexecve(const char *path, const char **argp, const char **envp);
int task_execve(task_t *dst, const char *path, const char **argp, const char **envp);

task_ctl_t *task_ctl_create(void);
void task_ctl_free(task_ctl_t *ctl);

// Waiting and blocking stuff
void task_set_sleep(task_t *task, const struct timespec *ts);

void task_busy(task_t *task);
void task_nobusy(task_t *task);
//

task_t *task_create(void);
int task_init(task_t *t, task_entry_func entry, void *arg, uint32_t flags);
void task_destroy(task_t *t);

// void task_copy_to_user(task_t *t, userspace void *dst, const void *src, size_t siz);
// void task_copy_from_user(task_t *t, void *dst, const userspace void *src, size_t siz);

// TODO: move this to sched.h?
void task_enable();
task_t *task_by_pid(int pid);
