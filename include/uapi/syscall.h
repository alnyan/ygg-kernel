#pragma once
#include <stdint.h>
#include <stddef.h>
#include "time.h"

#define userspace
typedef int ssize_t;
typedef void task_t;
// TODO: export this separately
typedef int pid_t;

#define SYSCALL_NR_EXIT         0x01
#define SYSCALL_NR_FORK         0x02

#define SYSCALL_NR_READ         0x03
#define SYSCALL_NR_WRITE        0x04
#define SYSCALL_NR_OPEN         0x05
#define SYSCALL_NR_CLOSE        0x06

#define SYSCALL_NR_WAITPID      0x07
#define SYSCALL_NR_EXECVE       0x0B
#define SYSCALL_NRX_FEXECVE     0x11

#define SYSCALL_NR_GETPID       0x14

#define SYSCALL_NR_KILL         0x25

#define SYSCALL_NR_NANOSLEEP    0xA2

// The implementation is somewhat different from linux kernel's: it has only one signal entry point
// for userspace libc
#define SYSCALL_NRX_SIGNAL      0x30
#define SYSCALL_NRX_SIGRETURN   0x77
