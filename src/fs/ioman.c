#include "ioman.h"
#include "sys/task.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "sys/mem.h"
#include <stddef.h>

#include "vfs.h"
#include "dev/tty.h"

static void ioman_task(void *arg) {
    vfs_fd_t fd;
    vfs_opendev(&fd, dev_tty, 0);
    fd.task = x86_task_current;
    char c;

    vfs_write(&fd, "> ", 2);
    while (1) {
        vfs_read(&fd, &c, 1);

        if (c == '\n') {
            break;
        } else {
            vfs_write(&fd, &c, 1);
        }
    }

    kdebug("Done\n");
    while (1);
}

static task_t *ioman_task_obj;

void ioman_init(void) {
    ioman_task_obj = task_create();
    task_set_kernel(ioman_task_obj, (task_entry_func) ioman_task, NULL, 0);
}

void ioman_start_task(void) {
    task_enable(ioman_task_obj);
}

ssize_t ioman_dev_read(dev_t *dev, task_t *task, void *buf, uintptr_t pos, size_t req) {
    if (dev->would_block(dev, pos, req, 0)) {
        ssize_t res = 0;
        ioman_op_t op = {
            dev,
            0,
            pos,
            buf,
            &res,
            req,
            task
        };

        assert(dev && dev->read);
        ((struct x86_task *) task)->flag |= TASK_FLG_BUSY;
        dev->read(dev, &op);

        asm volatile ("sti");
        while (((struct x86_task *) task)->flag & TASK_FLG_BUSY) {
            asm volatile ("hlt");
        }
        asm volatile ("cli");

        return res;
    } else {
        panic("NYI\n");
    }
}

ssize_t ioman_dev_write(dev_t *dev, task_t *task, const void *buf, uintptr_t pos, size_t req) {
    if (dev->would_block(dev, pos, req, 1)) {
        panic("NYI\n");
    } else {
        return dev->write_imm(dev, buf, pos, req);
    }
}

int ioman_op_signal_data(ioman_op_t *op, void *src, ssize_t count) {
    kdebug("Signal data\n");

    if (count < 0) {
        assert(op->res && op->task);
        *(op->res) = count;
        ((struct x86_task *) (op->task))->flag &= ~TASK_FLG_BUSY;
        return 1;
    }

    // Device provided more than we requested
    ssize_t siz = count;
    if (siz > op->req) {
        siz = op->req;
    }
    size_t pos = *(op->res);

    memcpy((void *) ((uintptr_t) op->buf + pos), src, siz);

    *(op->res) += siz;
    op->req -= siz;

    if (op->req == 0) {
        ((struct x86_task *) (op->task))->flag &= ~TASK_FLG_BUSY;
        return 1;
    }

    return 0;
}
