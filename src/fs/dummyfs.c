#include "dummyfs.h"
#include "sys/string.h"
#include "sys/debug.h"
#include "dev/tty.h"

static struct vfs dummyfs;
static vfs_node_t *node_tty;

vfs_t *vfs_dummyfs = &dummyfs;

static vfs_node_t *dummyfs_find_node(vfs_t *fs, const char *path) {
    kdebug("find_node %s\n", path);
    if (!strcmp("/tty0", path)) {
        return node_tty;
    }
    return NULL;
}

void dummyfs_init(void) {
    vfs_init(vfs_dummyfs);

    dummyfs.find_node = dummyfs_find_node;

    // Create basic nodes
    node_tty = vfs_node_create();
    node_tty->flags = VFS_NODE_TYPE_CHR;
    node_tty->fd_dev.dev = dev_tty;
}
