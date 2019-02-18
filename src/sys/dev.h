#pragma once
#include <stddef.h>
#include <stdint.h>

#define DEV_FLG_CHR     (1 << 0)    // Block device otherwise
#define DEV_FLG_RD      (1 << 1)
#define DEV_FLG_WR      (1 << 2)

typedef int ssize_t;

typedef struct vfs_file vfs_file_t;
typedef struct dev dev_t;

typedef ssize_t (*dev_read_func)(dev_t *, vfs_file_t *, void *, size_t, uint32_t);
typedef ssize_t (*dev_write_func)(dev_t *, vfs_file_t *, const void *, size_t, uint32_t);
typedef int (*dev_open_func)(dev_t *, vfs_file_t *, uint32_t);
typedef void (*dev_close_func)(dev_t *, vfs_file_t *, uint32_t);

struct dev {
    uint32_t flags;

    dev_open_func open;
    dev_close_func close;
    dev_read_func read;
    dev_write_func write;
};

////

void dev_init(dev_t *dev, uint32_t flags);

////

int dev_open(dev_t *dev, vfs_file_t *f, uint32_t flags);
