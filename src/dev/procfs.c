#include "procfs.h"
#include "sys/atoi.h"
#include "sys/task.h"
#include "sys/string.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "sys/heap.h"

// When pos0 set in dir, tells readdir we're looking through pids in pos1
#define PROCFS_DIR_ROOT_PIDS        0
// Iterating static nodes
#define PROCFS_DIR_ROOT_NODES       1
#define PROCFS_DIR_ROOT_END         2

typedef struct procfs_node procfs_node_t;

static vfs_t procfs;
vfs_t *vfs_procfs;

struct procfs_node {
    struct procfs_node *next;
    char name[255];
    uint32_t flags;
    void *data;

    union {
        procfs_get_int_func get_int;
        procfs_get_long_func get_long;
    };
};

static struct procfs_node *procfs_node_first;
static struct procfs_node *procfs_node_last;

// x86 proc control
#if defined(ARCH_X86)
#include "arch/x86/task/task.h"
#endif

static int procfs_opendir(vfs_t *fs, vfs_mount_t *mnt, vfs_dir_t *dir, const char *path, uint32_t flags) {
    // Root of procfs
    if (*path == 0) {
        dir->pos0 = PROCFS_DIR_ROOT_PIDS;
#if defined(ARCH_X86)
        dir->pos1 = (uintptr_t) x86_task_first;
#endif
        return 0;
    }

    // Check if it's a pid
    const char *pid_sub = NULL;
    int m = 1;
    for (const char *x = path; *x; ++x) {
        if (*x == '/') {
            // Store subdirectory of PID
            pid_sub = x + 1;
            break;
        }
        if (*x < '0' || *x > '9') {
            m = 0;
            break;
        }
    }

    if (m) {
        int pid = atoi(path);
        task_t *t = task_by_pid(pid);

        if (!t) {
            return -1;
        }

        return -1;
    }

    return -1;
}

static int procfs_readdir(vfs_t *fs, vfs_dir_t *dir, vfs_dirent_t *ent, uint32_t flags) {
    // Iterating pids
    if (dir->pos0 == PROCFS_DIR_ROOT_PIDS) {
        assert(dir->pos1);

        // Fill pid entry
#if defined(ARCH_X86)
        struct x86_task *t = (struct x86_task *) dir->pos1;
        if (t->ctl) {
            debug_ds(t->ctl->pid, ent->d_name, 0, 0);
        } else {
            strcpy(ent->d_name, "0");
        }
        ent->d_type = VFS_DT_DIR;

        t = t->next;
        if (t) {
            dir->pos1 = (uintptr_t) t;
        } else {
            if (procfs_node_first) {
                dir->pos0 = PROCFS_DIR_ROOT_NODES;
                dir->pos1 = (uintptr_t) procfs_node_first;
            } else {
                dir->pos0 = PROCFS_DIR_ROOT_END;
            }
        }
#endif
        return 0;
    } else if (dir->pos0 == PROCFS_DIR_ROOT_NODES) {
        assert(dir->pos1);

        struct procfs_node *node = (struct procfs_node *) dir->pos1;

        strcpy(ent->d_name, node->name);
        // TODO: nested nodes
        ent->d_type = VFS_DT_REG;

        node = node->next;
        if (node) {
            dir->pos1 = (uintptr_t) node;
        } else {
            dir->pos1 = 0;
            dir->pos0 = PROCFS_DIR_ROOT_END;
        }

        return 0;
    }
    return -1;
}

static int procfs_open(vfs_t *fs, vfs_file_t *f, const char *path, uint32_t flags) {
    // Check if it's a pid
    const char *pid_sub = NULL;
    int m = 1;
    for (const char *x = path; *x; ++x) {
        if (*x == '/') {
            // Store subdirectory of PID
            pid_sub = x + 1;
            break;
        }
        if (*x < '0' || *x > '9') {
            m = 0;
            break;
        }
    }

    if (!m) {
        for (struct procfs_node *node = procfs_node_first; node; node = node->next) {
            if (!strcmp(node->name, path)) {
                f->fs_priv = node;
                f->pos0 = 0;
                f->pos1 = (node->flags & 0xFF) | (1 << 31);
                return 0;
            }
        }
    } else {
        // TODO: implement process info
    }
    return -1;
}

static ssize_t procfs_read(vfs_t *fs, vfs_file_t *f, void *buf, size_t lim, uint32_t flags) {
    assert(f);
    assert(f->fs_priv);
    if (f->pos1 & (1 << 31)) {
        size_t cnt = ((f->pos1 & 0xFF) > lim) ? lim : (f->pos1 & 0xFF);
        struct procfs_node *node = (struct procfs_node *) f->fs_priv;

        if (cnt == 0) {
            return -1;
        }

        if ((node->flags & 0xFF) == 4) {
            // Int
            int r = node->get_int(node->data);
            uintptr_t addr = (uintptr_t) &r + f->pos0;
            memcpy(buf, (const void *) addr, cnt);
            f->pos1 = (f->pos1 & 0xFFFFFF00) | ((f->pos1 & 0xFF) - cnt);
            f->pos0 += cnt;

            return cnt;
        }

        return -1;
    }
    return -1;
}

void procfs_add_int(const char *name, procfs_get_int_func f, void *v) {
    struct procfs_node *node = (struct procfs_node *) heap_alloc(sizeof(struct procfs_node));

    strcpy(node->name, name);
    node->next = NULL;
    node->get_int = f;
    node->data = v;
    node->flags = sizeof(int);

    if (procfs_node_last) {
        procfs_node_last->next = node;
    } else {
        procfs_node_first = node;
    }

    procfs_node_last = node;
}

static int procfs_get_test(void *v) {
    return 1234;
}

void procfs_init(void) {
    kinfo("Initializing procfs\n");

    vfs_procfs = &procfs;
    vfs_init(vfs_procfs);

    vfs_procfs->flags = VFS_FLG_RD;
    vfs_procfs->opendir = procfs_opendir;
    vfs_procfs->readdir = procfs_readdir;

    vfs_procfs->open = procfs_open;
    vfs_procfs->read = procfs_read;

    procfs_node_first = NULL;
    procfs_node_last = NULL;

    // Add "test" node
    procfs_add_int("test", procfs_get_test, NULL);
}
