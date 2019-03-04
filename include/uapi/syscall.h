#pragma once
#include <stdint.h>
#include <stddef.h>

#define userspace

#define SYSCALL_DECL0(name)                 int sys_##name(void)
#define SYSCALL_DEFINE0(name)               int sys_##name(void)
#define SYSCALL_DECL1(name, x)              int sys_##name(x)
#define SYSCALL_DEFINE1(name, x)            int sys_##name(x)
#define SYSCALL_DECL2(name, x, y)           int sys_##name(x, y)
#define SYSCALL_DEFINE2(name, x, y)         int sys_##name(x, y)
#define SYSCALL_DECL3(name, x, y, z)        int sys_##name(x, y, z)
#define SYSCALL_DEFINE3(name, x, y, z)      int sys_##name(x, y, z)
#define SYSCALL_DECL4(name, x, y, z, w)     int sys_##name(x, y, z, w)
#define SYSCALL_DEFINE4(name, x, y, z, w)   int sys_##name(x, y, z, w)

#define SYSOP_ASYNC             0x01
#define SYSOP_SYNC              0x00

#define SYSCALL_NR_EXIT         0x01
SYSCALL_DECL1(exit, int);
#define SYSCALL_NR_FORK         0x02
SYSCALL_DECL0(fork);

#define SYSCALL_NR_READ         0x03
SYSCALL_DECL4(read, int, userspace void *, size_t, ssize_t *);
#define SYSCALL_NR_WRITE        0x04
SYSCALL_DECL3(write, int, const userspace void *, size_t);
#define SYSCALL_NR_OPEN         0x05
SYSCALL_DECL3(open, const userspace char *, int, uint32_t);
#define SYSCALL_NR_CLOSE        0x06
SYSCALL_DECL1(close, int);

#define SYSCALL_NR_EXECVE       0x0B
SYSCALL_DECL3(execve, const userspace char *, const userspace char **, const userspace char **);
#define SYSCALL_NRX_FEXECVE     0x11
SYSCALL_DECL3(fexecve, const userspace char *, const userspace char **, const userspace char **);

#define SYSCALL_NR_GETPID       0x14
SYSCALL_DECL0(getpid);

#define SYSCALL_NR_NANOSLEEP    0xA2
SYSCALL_DECL1(nanosleep, const userspace struct timespec *);

// The implementation is somewhat different from linux kernel's: it has only one signal entry point
// for userspace libc
#define SYSCALL_NRX_SIGNAL      0x30
SYSCALL_DECL2(signal, int, userspace void(*)(int));
#define SYSCALL_NRX_RAISE       0x31
SYSCALL_DECL1(raise, int);
