#include "vfs.h"
#include "sys/assert.h"
#include "fs/ioman.h"
#include <uapi/errno.h>

int vfs_opendev(vfs_fd_t *fd, dev_t *dev, uint32_t flags) {
    assert(fd);

    // TODO: get this information from device struct
    fd->flags = (flags & ~0x1) | VFS_FD_TYPE_CHR;

    fd->fd_dev.pos = 0;
    fd->fd_dev.dev = dev;

    return 0;
}

ssize_t vfs_read(vfs_fd_t *fd, void *buf, size_t count) {
    assert(fd && fd->task);

    switch (VFS_FD_GET_TYPE(fd)) {
    case VFS_FD_TYPE_BLK:
    case VFS_FD_TYPE_CHR:
        assert(fd->fd_dev.dev);
        return ioman_dev_read(fd->fd_dev.dev, fd->task, buf, fd->fd_dev.pos, count);
    default:
        panic("Invalid FD\n");
        return -EBADF;
    }

    return -1;
}

ssize_t vfs_write(vfs_fd_t *fd, const void *buf, size_t count) {
    assert(fd && fd->task);

    switch (VFS_FD_GET_TYPE(fd)) {
    case VFS_FD_TYPE_BLK:
    case VFS_FD_TYPE_CHR:
        assert(fd->fd_dev.dev);
        return ioman_dev_write(fd->fd_dev.dev, fd->task, buf, fd->fd_dev.pos, count);
    default:
        panic("Bad FD\n");
        return -EBADF;
    }
}
