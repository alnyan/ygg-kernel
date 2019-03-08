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

int ioman_op_signal_data(ioman_op_t *op, void *src, ssize_t count);
