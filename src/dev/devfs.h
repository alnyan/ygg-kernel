#pragma once
#include "sys/vfs.h"

extern vfs_t *vfs_devfs;

void devfs_init(void);
void devfs_populate(void);
void devfs_add(dev_t *dev, const char *path, uintptr_t param);
