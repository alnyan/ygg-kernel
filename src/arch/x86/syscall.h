#pragma once
#include <uapi/syscall.h>

#define SYSCALL_DECL0(name)                 int sys_##name(task_t *)
#define SYSCALL_DEFINE0(name)               int sys_##name(task_t *task)
#define SYSCALL_DECL1(name, x)              int sys_##name(task_t *, x)
#define SYSCALL_DEFINE1(name, x)            int sys_##name(task_t *task, x)
#define SYSCALL_DECL2(name, x, y)           int sys_##name(task_t *, x, y)
#define SYSCALL_DEFINE2(name, x, y)         int sys_##name(task_t *task, x, y)
#define SYSCALL_DECL3(name, x, y, z)        int sys_##name(task_t *, x, y, z)
#define SYSCALL_DEFINE3(name, x, y, z)      int sys_##name(task_t *task, x, y, z)
#define SYSCALL_DECL4(name, x, y, z, w)     int sys_##name(task_t *, x, y, z, w)
#define SYSCALL_DEFINE4(name, x, y, z, w)   int sys_##name(task_t *task, x, y, z, w)

SYSCALL_DECL1(exit, int);
SYSCALL_DECL0(fork);

SYSCALL_DECL3(read, int, userspace void *, size_t);
SYSCALL_DECL3(write, int, const userspace void *, size_t);
SYSCALL_DECL3(open, const userspace char *, int, uint32_t);
SYSCALL_DECL1(close, int);

SYSCALL_DECL3(waitpid, pid_t, userspace int *, int);
SYSCALL_DECL3(execve, const userspace char *, const userspace char **, const userspace char **);
SYSCALL_DECL3(fexecve, const userspace char *, const userspace char **, const userspace char **);

SYSCALL_DECL0(getpid);

SYSCALL_DECL2(kill, pid_t, int);

SYSCALL_DECL1(nanosleep, const userspace struct timespec *);

SYSCALL_DECL2(signal, int, userspace void(*)(int));
SYSCALL_DECL0(sigreturn);
