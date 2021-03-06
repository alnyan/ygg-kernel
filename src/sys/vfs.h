#pragma once
#include <stddef.h>
#include <stdint.h>

#define VFS_FLG_RD      (1 << 0)
#define VFS_FLG_WR      (1 << 1)
#define VFS_FLG_DIR     (1 << 21)

#define VFS_TYPE_REG    0
#define VFS_TYPE_BLK    1
#define VFS_TYPE_CHR    2
#define VFS_TYPE_SOCK   3
#define VFS_TYPE_DIR    4

#define VFS_DT_CHR      2
#define VFS_DT_DIR      4
#define VFS_DT_BLK      6
#define VFS_DT_REG      8

#define VFS_READ_ASYNC  ((ssize_t) -2)

#define VFS_FACT_STAT   1
#define VFS_FACT_BLKDEV 2   // Get block device associated with path
#define VFS_FACT_GETM   3   // Returns pointer to memory storage, if the file is linear and stored
                            //  in memory

typedef int ssize_t;

typedef struct dev dev_t;
typedef struct vfs vfs_t;

typedef struct vfs_mount vfs_mount_t;
typedef int (*vfs_mount_func)(vfs_t *, vfs_mount_t *, uint32_t);
typedef int (*vfs_umount_func)(vfs_t *, vfs_mount_t *, uint32_t);

// TODO: use vfs_mount_t instead of vfs_t, as it provides more context
typedef struct vfs_file vfs_file_t;
typedef int (*vfs_open_func)(vfs_t *, vfs_file_t *, const char *, uint32_t);
typedef void (*vfs_close_func)(vfs_t *, vfs_file_t *, uint32_t);
typedef ssize_t (*vfs_write_func)(vfs_t *, vfs_file_t *, const void *, size_t, uint32_t);
typedef ssize_t (*vfs_read_func)(vfs_t *, vfs_file_t *, void *, size_t, uint32_t);
// Combination of stat/link/etc.
typedef int (*vfs_fact_func)(vfs_t *, vfs_mount_t *, const char *, uint32_t, ...);

typedef struct vfs_file vfs_dir_t;
typedef struct vfs_dirent vfs_dirent_t;
typedef int (*vfs_opendir_func)(vfs_t *, vfs_mount_t *, vfs_dir_t *, const char *, uint32_t);
typedef void (*vfs_closedir_func)(vfs_t *, vfs_dir_t *, uint32_t);
typedef int (*vfs_readdir_func)(vfs_t *, vfs_dir_t *, vfs_dirent_t *, uint32_t);
// Combination of mkdir/rmdir/etc.
typedef int (*vfs_diract_func)(vfs_t *, const char *, uint32_t, ...);

struct vfs_stat {
    uint32_t flags;
};

struct vfs_dirent {
    uint32_t d_ino;
    uint32_t d_off;
    uint16_t d_reclen;
    uint8_t d_type;
    char d_name[256];
};

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
    vfs_fact_func fact;

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
    char path[256];
    // Device-specific info
    void *dev_priv, *fs_priv;
    uintptr_t pos0, pos1;

    // Pending operation info
    void *op_buf;
    ssize_t *op_res;
    size_t op_len;
    int op_type;
    void *task;
};

struct vfs_mount {
    uint32_t flags;
    vfs_t *fs;
    dev_t *srcdev;
    char src[256];
    char dst[256];
    void *fs_priv;
};

////

void vfs_init(vfs_t *fs);

////

vfs_file_t *vfs_open(const char *path, uint32_t flags);
void vfs_close(vfs_file_t *f);
ssize_t vfs_read(vfs_file_t *f, void *buf, size_t len, ssize_t *res);
ssize_t vfs_write(vfs_file_t *f, const void *buf, size_t len);

// Helper wrapper around vfs_read
ssize_t vfs_gets(vfs_file_t *f, char *buf, size_t len);

int vfs_stat(const char *path, struct vfs_stat *st);
dev_t *vfs_get_blkdev(const char *path);
uintptr_t vfs_getm(const char *path);

vfs_dir_t *vfs_opendir(const char *path);
int vfs_readdir(vfs_dir_t *dir, vfs_dirent_t *ent);
void vfs_closedir(vfs_dir_t *dir);

void vfs_dirent_dump(const vfs_dirent_t *ent);

////

int vfs_mount(const char *src, const char *dst, vfs_t *fs_type, uint32_t flags);
int vfs_umount(const char *dst, uint32_t flags);

////

int vfs_lookup_file(const char *path, vfs_mount_t **fs, const char **relpath);

////

int vfs_send_read_res(vfs_file_t *f, const void *src, size_t count);
