#pragma once
#include "sys/task.h"
#include "dev/dev.h"
#include <stdint.h>
#include <stddef.h>

typedef int ssize_t;
typedef struct ioman_op ioman_op_t;

struct ioman_op {
    dev_t *dev;
    uint32_t flags;
    uintptr_t position;

    void *buf;
    ssize_t *res;
    size_t req;

    task_t *task;
    // TODO: store file descriptor
};

void ioman_init(void);
void ioman_start_task(void);

ssize_t ioman_dev_read(dev_t *dev, task_t *task, void *buf, uintptr_t pos, size_t count);
ssize_t ioman_dev_write(dev_t *dev, task_t *task, const void *buf, uintptr_t pos, size_t count);

void ioman_op_signal_error(ioman_op_t *op, int err);
void ioman_op_signal_success(ioman_op_t *op);
int ioman_buf_read(ioman_op_t *op, void *dst, ssize_t count, ssize_t *res);
int ioman_buf_write(ioman_op_t *op, const void *src, ssize_t count, ssize_t *res);
