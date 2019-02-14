#pragma once
#include <stddef.h>
#include <stdint.h>
#include "device.h"

typedef struct vfs_file vfs_file_t;
struct vfs_file {
    // Device pointer
    dev_t *f_dev;
    void *f_dev_info;
    // Task the file is bound to
    void *f_task;
    uint32_t flags;

    // Userspace-stored
    char *buf;
    size_t reqw;
    size_t reqr;
};


vfs_file_t *vfs_alloc(void);
int vfs_open(vfs_file_t *f, const char *path, int fl);
void vfs_bind(vfs_file_t *f, void *task);
int vfs_read(vfs_file_t *f, void *buf, size_t sz);

// Add pending IO read operation
void io_pending_read_set(vfs_file_t *f, void *buf, size_t n);
// Signal that some data arrived
void io_pending_read_add(vfs_file_t *f, const void *buf, size_t n);
// Select a pending operation for device and address
vfs_file_t *io_pending_read_first(dev_t *dev, uintptr_t a);
