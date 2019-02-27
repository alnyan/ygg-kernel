#pragma once
#include "sys/vfs.h"

typedef int (*procfs_get_int_func)(void *);
typedef long (*procfs_get_long_func)(void *);

extern vfs_t *vfs_procfs;

void procfs_add_int(const char *name, procfs_get_int_func func, void *data);
void procfs_add_long(const char *name, procfs_get_long_func func, void *data);
void procfs_init(void);
