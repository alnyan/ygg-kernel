#pragma once
#include <stdint.h>
typedef struct vfs_node vfs_node_t;
#include "sys/mm.h"

typedef void task_t;
typedef int pid_t;

#if defined(ARCH_X86)
#include "arch/x86/task/task.h"
#else
#error "Not supported"
#endif

#define TASK_FLG_RUNNING            (1 << 0)
#define TASK_FLG_STOP               (1 << 1)
#define TASK_FLG_WAIT               (1 << 2)
#define TASK_FLG_BUSY               (1 << 3)
#define TASK_FLG_SIG                (1 << 4)
#define TASK_FLG_KERNEL             (1 << 31)

// General control
struct task_ctl {
    // General
    char name[64];
    task_t *parent;
    pid_t pid;
    uint32_t flags;

    // Wait and sleep
    union {
        uint64_t wait_sleep;
        pid_t wait_pid;
    } wait;

    // I/O
    vfs_node_t *fds[4];

    // Memory
    mm_space_t space;       // current page directory
    uintptr_t image_end;    // usable memory after the image, page aligned
    uintptr_t brk;          // (brk - image_end) is heap size

    // Signal
    int sigq[4];            // task's signal queue
};

void task_init(void);

task_t *task_create(void);
task_ctl_t *task_ctl_create(void);
void task_destroy(task_t *t, uint32_t flags);
#ifndef task_ctl
task_ctl_t *task_ctl(task_t *t);
#endif

vfs_node_t *task_get_fd(task_t *t, int fd);
int task_bind_fd(task_t *t, vfs_node_t *node);
void task_ubind_fd(task_t *t, int fd);

#ifndef task_space
mm_space_t task_space(task_t *t);
#endif

void task_busy(task_t *t);

int task_fexecve(const char *p, const char **argp, const char **envp);
void task_terminate(task_t *t, int signum);
