#pragma once
#include <stddef.h>
#include <stdint.h>

typedef int ssize_t;

typedef struct dev dev_t;
typedef struct vfs vfs_t;

typedef struct vfs_mount vfs_mount_t;
typedef int (*vfs_mount_func)(vfs_t *, vfs_mount_t *, uint32_t);
typedef int (*vfs_umount_func)(vfs_t *, vfs_mount_t *, uint32_t);

typedef struct vfs_file vfs_file_t;
typedef int (*vfs_open_func)(vfs_t *, vfs_file_t *, const char *, uint32_t);
typedef void (*vfs_close_func)(vfs_t *, vfs_file_t *, uint32_t);
typedef ssize_t (*vfs_write_func)(vfs_t *, vfs_file_t *, const void *, size_t, uint32_t);
typedef ssize_t (*vfs_read_func)(vfs_t *, vfs_file_t *, void *, size_t, uint32_t);

typedef struct vfs_dir vfs_dir_t;
typedef struct vfs_dirent vfs_dirent_t;
typedef int (*vfs_opendir_func)(vfs_t *, vfs_dir_t *, const char *, uint32_t);
typedef void (*vfs_closedir_func)(vfs_t *, vfs_dir_t *, uint32_t);
typedef int (*vfs_readdir_func)(vfs_t *, vfs_dir_t *, vfs_dirent_t *, uint32_t);
// Combination of mkdir/rmdir/etc.
typedef int (*vfs_diract_func)(vfs_t *, const char *, uint32_t);

struct vfs {
    uint32_t flags;

    // Mount ops
    vfs_mount_func mount;
    vfs_umount_func umount;

    // File ops
    vfs_open_func open;
    vfs_close_func close;
    vfs_read_func read;
    vfs_write_func write;

    // Dir ops
    vfs_opendir_func opendir;
    vfs_closedir_func closedir;
    vfs_readdir_func readdir;
    vfs_diract_func diract;
};

struct vfs_file {
    uint32_t flags;
    dev_t *dev;
    vfs_t *fs;
    // Device-specific info
    void *dev_priv;
    // FS-specific info
    void *fs_priv;
};

struct vfs_mount {
    uint32_t flags;
    vfs_t *fs;
    char src[256];
    char dst[256];
    void *fs_priv;
};

////

void vfs_init(vfs_t *fs);

////

vfs_file_t *vfs_open(const char *path, uint32_t flags);
void vfs_close(vfs_file_t *f);
ssize_t vfs_read(vfs_file_t *f, void *buf, size_t len);
ssize_t vfs_write(vfs_file_t *f, const void *buf, size_t len);

////

int vfs_mount(const char *src, const char *dst, vfs_t *fs_type, uint32_t flags);
int vfs_umount(const char *dst, uint32_t flags);

////

int vfs_lookup_file(const char *path, vfs_mount_t **fs, const char **relpath);
