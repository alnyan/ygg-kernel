#include "vfs.h"
#include "sys/task.h"
#include "sys/string.h"
#include "sys/panic.h"
#include "sys/debug.h"
#include "arch/x86/task.h"

static struct vfs_file s_files[64];
static vfs_file_t *s_pending_reads[128] = { 0 };

vfs_file_t *vfs_alloc(void) {
    for (int i = 0; i < 64; ++i) {
        if (!(s_files[i].flags & 1)) {
            s_files[i].flags |= 1;
            return &s_files[i];
        }
    }
    return NULL;
}

void vfs_bind(vfs_file_t *f, void *task) {
    f->f_task = task;
}

int vfs_open(vfs_file_t *f, const char *path, int flags) {
    // It's a testing feature, so only kb is supported now
    if (!strcmp(path, "kb")) {
        extern dev_t *dev_keyboard;
        f->f_dev = dev_keyboard;
        return -1;
    }

    return -1;
}

int vfs_read(vfs_file_t *f, void *buf, size_t lim) {
    if (f->f_dev && (f->f_dev->flags & DEV_FLG_READ)) {
        return f->f_dev->read(f->f_dev, f, buf, lim);
    }
    return -1;
}

void io_pending_read_set(vfs_file_t *f, void *buf, size_t sz) {
    if (f->f_task) {
        task_busy(f->f_task);
    }

    f->buf = buf;
    f->reqr = sz;

    for (int i = 0; i < sizeof(s_pending_reads) / sizeof(s_pending_reads[0]); ++i) {
        if (!s_pending_reads[i]) {
            s_pending_reads[i] = f;
            break;
        }
    }
}

vfs_file_t *io_pending_read_first(dev_t *dev, uintptr_t addr) {
    // addr is ignore now
    // XXX: possible bug, we can fulfil pending read of some other task reading from the same loc
    for (int i = 0; i < sizeof(s_pending_reads) / sizeof(s_pending_reads[0]); ++i) {
        if (s_pending_reads[i] && s_pending_reads[i]->f_dev == dev) {
            return s_pending_reads[i];
        }
    }

    return NULL;
}

void io_pending_read_add(vfs_file_t *f, const void *data, size_t n) {
    size_t l = n > f->reqr ? f->reqr : n;
    f->reqr -= l;

    // Switch to the task's page dir and copy the data there
    // XXX: this is very plaform-specific
    uint32_t cr3 = *((uint32_t *) (((struct x86_task *) f->f_task)->ebp0 - 14 * 4));
    asm volatile ("movl %%eax, %%cr3"::"a"(cr3));
    memcpy(f->buf, data, l);
    cr3 = ((uintptr_t) mm_current) - 0xC0000000;
    asm volatile ("movl %%eax, %%cr3"::"a"(cr3):"memory");

    if (!f->reqr) {
        f->buf = 0;

        task_nobusy(f->f_task);
    }
}
