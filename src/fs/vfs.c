#include "vfs.h"
#include "sys/assert.h"
#include "fs/ioman.h"
#include "sys/heap.h"
#include "sys/string.h"
#include "sys/mem.h"
#include <uapi/errno.h>

// The leaf nodes of the VFS tree are always mountpoints. Any paths relative to them are handled by
// filesystem drivers.
static vfs_node_t *vfs_root = NULL;

void vfs_init(vfs_t *fs) {
    memset(fs, 0, sizeof(vfs_t));
}

vfs_node_t *vfs_node_create(void) {
    vfs_node_t *node = (vfs_node_t *) heap_alloc(sizeof(vfs_node_t));
    if (!node) {
        return node;
    }

    memset(node, 0, sizeof(vfs_node_t));
    return node;
}

vfs_node_t *vfs_find_node(const char *path) {
    // const char *p = path, *e;
    // TODO: canonicalize the path

    if (!vfs_root) {
        return NULL;
    }

    assert(VFS_NODE_GET_TYPE(vfs_root) == VFS_NODE_TYPE_MNT);

    return vfs_root->fd_mount.fs->find_node(vfs_root->fd_mount.fs, path);
}

vfs_node_t *vfs_mount_path(const char *path, const char *src, vfs_t *fs, uint32_t opt) {
    vfs_node_t *node = NULL;

    if (!*path || !strcmp(path, "/")) {
        // Create a root node
        vfs_root = vfs_node_create();
        vfs_root->flags = VFS_NODE_TYPE_MNT;
        // Empty name
        vfs_root->name[0] = 0;
        node = vfs_root;
    }

    if (!node) {
        return NULL;
    }

    node->fd_mount.src[255] = 0;
    if (src) {
        strncpy(node->fd_mount.src, src, 255);
    } else {
        node->fd_mount.src[0] = 0;
    }

    node->fd_mount.fs = fs;

    return node;
}

int vfs_opendev(vfs_node_t *fd, dev_t *dev, uint32_t flags) {
    assert(fd);

    // TODO: get this information from device struct
    fd->flags = (flags & ~0x1) | VFS_NODE_TYPE_CHR;

    fd->fd_dev.pos = 0;
    fd->fd_dev.dev = dev;

    return 0;
}

ssize_t vfs_read(vfs_node_t *fd, void *buf, size_t count) {
    assert(fd && fd->task);

    switch (VFS_NODE_GET_TYPE(fd)) {
    case VFS_NODE_TYPE_BLK:
    case VFS_NODE_TYPE_CHR:
        assert(fd->fd_dev.dev);
        return ioman_dev_read(fd->fd_dev.dev, fd->task, buf, fd->fd_dev.pos, count);
    default:
        panic("Invalid FD\n");
        return -EBADF;
    }

    return -1;
}

ssize_t vfs_write(vfs_node_t *fd, const void *buf, size_t count) {
    assert(fd && fd->task);

    switch (VFS_NODE_GET_TYPE(fd)) {
    case VFS_NODE_TYPE_BLK:
    case VFS_NODE_TYPE_CHR:
        assert(fd->fd_dev.dev);
        return ioman_dev_write(fd->fd_dev.dev, fd->task, buf, fd->fd_dev.pos, count);
    default:
        panic("Bad FD\n");
        return -EBADF;
    }
}
