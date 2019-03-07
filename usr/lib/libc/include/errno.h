#pragma once
#include <uapi/errno.h>

extern int errno;

const char *strerror(int e);
void perror(const char *v);
