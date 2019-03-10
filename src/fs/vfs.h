#pragma once
#include <stdint.h>
#include <stddef.h>
#include "sys/task.h"
#include "dev/dev.h"
#include "sys/list.h"

typedef int ssize_t;

typedef struct vfs vfs_t;
typedef struct vfs_node vfs_node_t;

////
// Filesystem functions

typedef vfs_node_t *(*fs_find_node_func) (vfs_t *, task_t *, const char *);
typedef ssize_t (*fs_read_func) (vfs_t *, vfs_node_t *, void *, size_t);
typedef ssize_t (*fs_write_func) (vfs_t *, vfs_node_t *, const void *, size_t);
typedef uintptr_t (*fs_getm) (vfs_t *, vfs_node_t *);

////
// File descriptor
#define VFS_NODE_GET_TYPE(fd)     ((fd)->flags & 0xF)
#define VFS_NODE_TYPE_REG         0x0
#define VFS_NODE_TYPE_DIR         0x1
#define VFS_NODE_TYPE_BLK         0x2
#define VFS_NODE_TYPE_CHR         0x3
#define VFS_NODE_TYPE_MNT         0xF
#define VFS_NODE_FLG_WR           (1 << 4)
#define VFS_NODE_FLG_RD           (1 << 5)
#define VFS_NODE_FLG_MEMR         (1 << 31)

struct vfs_node_dev {
    dev_t *dev;
    uintptr_t pos;
};

struct vfs_node_reg {
    vfs_t *fs;
    uintptr_t inode;
    uintptr_t off;
    uintptr_t size;
};

struct vfs_node_mount {
    vfs_t *fs;
    char src[256];
};

LIST(vfs_node_list, vfs_node_t *);

struct vfs_node {
    uint32_t flags;
    task_t *task;
    char name[256];
    vfs_node_list_t chilren;
    uint32_t refcount;

    union {
        struct vfs_node_dev fd_dev;
        struct vfs_node_reg fd_reg;
        struct vfs_node_mount fd_mount;
    };
};

////
// Filesystem

struct vfs {
    uint32_t flags;
    vfs_node_t *root_node;
    dev_t *dev;
    uintptr_t pos;

    fs_find_node_func find_node;
    fs_getm getm;

    fs_read_func read;
    fs_write_func write;
};

////
// VFS general functions

void vfs_init(vfs_t *fs);
vfs_node_t *vfs_node_create(void);
vfs_node_t *vfs_find_node(task_t *t, const char *path);
vfs_node_t *vfs_mount_path(const char *path, const char *src, vfs_t *fs, uint32_t opt);

// VFS FD functions

uintptr_t vfs_getm(vfs_node_t *fd);
int vfs_opendev(vfs_node_t *fd, dev_t *dev, uint32_t flags);
ssize_t vfs_read(vfs_node_t *fd, void *buf, size_t req);
ssize_t vfs_write(vfs_node_t *fd, const void *buf, size_t req);
