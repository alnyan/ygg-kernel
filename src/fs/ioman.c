#include "ioman.h"
#include "sys/task.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "sys/mem.h"
#include <stddef.h>
#include "sys/mm.h"

#include "vfs.h"
#include "dev/tty.h"
#include "fs/dummyfs.h"

static void ioman_task(void *arg) {
    while (1) {
        asm volatile ("hlt");
    }
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
        return dev->read_imm(dev, buf, pos, req);
    }
}

ssize_t ioman_dev_write(dev_t *dev, task_t *task, const void *buf, uintptr_t pos, size_t req) {
    if (dev->would_block(dev, pos, req, 1)) {
        ssize_t res = 0;
        ioman_op_t op = {
            dev,
            1,
            pos,
            (void *) buf,
            &res,
            req,
            task
        };

        assert(dev && dev->write);
        ((struct x86_task *) task)->flag |= TASK_FLG_BUSY;
        dev->write(dev, &op);

        asm volatile ("sti");
        while (((struct x86_task *) task)->flag & TASK_FLG_BUSY) {
            asm volatile ("hlt");
        }
        asm volatile ("cli");

        return res;
    } else {
        return dev->write_imm(dev, buf, pos, req);
    }
}

void ioman_op_signal_error(ioman_op_t *op, int err) {
    *(op->res) = err;
    op->req = 0;
    ((struct x86_task *) (op->task))->flag &= ~TASK_FLG_BUSY;
}

void ioman_op_signal_success(ioman_op_t *op) {
    op->req = 0;
    ((struct x86_task *) (op->task))->flag &= ~TASK_FLG_BUSY;
}

int ioman_buf_read(ioman_op_t *op, void *dst, ssize_t count, ssize_t *rd) {
    assert(count >= 0);
    assert(op->task && op->res);

    ssize_t siz = count;
    if (siz > op->req) {
        siz = op->req;
    }
    size_t pos = *(op->res);

    mm_memcpy_user_to_kernel(task_space(op->task), dst, (void *) ((uintptr_t) op->buf + pos), siz);

    *(op->res) += siz;
    op->req -= siz;
    if (rd) {
        *rd = siz;
    }

    if (op->req == 0) {
        ioman_op_signal_success(op);
    }

    return (op->req == 0);
}

int ioman_buf_write(ioman_op_t *op, const void *src, ssize_t count, ssize_t *wr) {
    assert(count >= 0);
    assert(op->task && op->res);

    ssize_t siz = count;
    if (siz > op->req) {
        siz = op->req;
    }
    size_t pos = *(op->res);

    mm_memcpy_kernel_to_user(task_space(op->task), (void *) ((uintptr_t) op->buf + pos), src, siz);

    *(op->res) += siz;
    op->req -= siz;
    if (wr) {
        *wr = siz;
    }

    if (op->req == 0) {
        ioman_op_signal_success(op);
    }

    return (op->req == 0);
}
