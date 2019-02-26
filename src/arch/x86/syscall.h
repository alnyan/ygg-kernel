#pragma once
#include <stddef.h>
#include <stdint.h>
#include "sys/attr.h"
#include "sys/time.h"

#define SYSCALL_DECL0(name)      int sys_##name(void)
#define SYSCALL_DEFINE0(name)    int sys_##name(void)
#define SYSCALL_DECL1(name, x)   int sys_##name(x)
#define SYSCALL_DEFINE1(name, x) int sys_##name(x)
#define SYSCALL_DECL3(name, x, y, z)   int sys_##name(x, y, z)
#define SYSCALL_DEFINE3(name, x, y, z) int sys_##name(x, y, z)

#define SYSOP_ASYNC             0x01
#define SYSOP_SYNC              0x00

#define SYSCALL_NR_EXIT         0x01
#define SYSCALL_NR_FORK         0x02
SYSCALL_DECL1(exit, int);
SYSCALL_DECL0(fork);

#define SYSCALL_NR_READ         0x03
#define SYSCALL_NR_WRITE        0x04
#define SYSCALL_NR_OPEN         0x05
#define SYSCALL_NR_CLOSE        0x06
// sys_write/sys_read are special and are defined specially
SYSCALL_DECL3(open, const userspace char *, int, uint32_t);
SYSCALL_DECL1(close, int);

#define SYSCALL_NR_EXECVE       0x0B
SYSCALL_DECL3(execve, const userspace char *, const userspace char **, const userspace char **);
#define SYSCALL_NRX_FEXECVE     0x11
SYSCALL_DECL3(fexecve, const userspace char *, const userspace char **, const userspace char **);

#define SYSCALL_NR_GETPID       0x14
SYSCALL_DECL0(getpid);

#define SYSCALL_NR_NANOSLEEP    0xA2
SYSCALL_DECL1(nanosleep, const userspace struct timespec *);
