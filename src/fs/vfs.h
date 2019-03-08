#pragma once
#include <stdint.h>
#include <stddef.h>
#include "sys/task.h"
#include "dev/dev.h"

typedef int ssize_t;

typedef struct vfs vfs_t;
typedef struct vfs_fd vfs_fd_t;

////
// Filesystem functions

typedef ssize_t (*fs_read_func) (vfs_t *, vfs_fd_t *, void *, size_t);
typedef ssize_t (*fs_write_func) (vfs_t *, vfs_fd_t *, const void *, size_t);

////
// File descriptor
#define VFS_FD_GET_TYPE(fd)     ((fd)->flags & 0xF)
#define VFS_FD_TYPE_REG         0x0
#define VFS_FD_TYPE_DIR         0x1
#define VFS_FD_TYPE_BLK         0x2
#define VFS_FD_TYPE_CHR         0x3
#define VFS_FD_FLG_WR           (1 << 4)

struct vfs_fd_dev {
    dev_t *dev;
    uintptr_t pos;
};

struct vfs_fd_reg {
    vfs_t *fs;
    uintptr_t inode;
    uintptr_t off;
};

struct vfs_fd {
    uint32_t flags;
    task_t *task;

    union {
        struct vfs_fd_dev fd_dev;
        struct vfs_fd_reg fd_reg;
    };
};

////
// VFS functions

int vfs_opendev(vfs_fd_t *fd, dev_t *dev, uint32_t flags);
ssize_t vfs_read(vfs_fd_t *fd, void *buf, size_t req);
ssize_t vfs_write(vfs_fd_t *fd, const void *buf, size_t req);
