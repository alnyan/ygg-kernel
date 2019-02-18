#pragma once
#include "sys/vfs.h"

extern vfs_t *vfs_devfs;

void devfs_init(void);
