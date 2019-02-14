#pragma once
#include <stddef.h>
#include <stdint.h>

#define SYSCALL_NR_WRITE    0x04
int sys_write(unsigned int fd, const char *buf, size_t len);
