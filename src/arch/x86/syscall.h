#pragma once
#include <stddef.h>
#include <stdint.h>

#define SYSCALL_DECL3(name, x, y, z)   int sys_##name(x, y, z)
#define SYSCALL_DEFINE3(name, x, y, z) int sys_##name(x, y, z)

#define SYSOP_ASYNC         0x01
#define SYSOP_SYNC          0x00

#define SYSCALL_NR_WRITE    0x04
SYSCALL_DECL3(write, int, const void *, size_t sz);

#define SYSCALL_NR_READ     0x03
SYSCALL_DECL3(read, int, void *, size_t sz);
