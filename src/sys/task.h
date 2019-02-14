#pragma once

#define TASK_FLG_RUNNING        (1 << 0)
#define TASK_FLG_STOP           (1 << 1)
#define TASK_FLG_WAIT           (1 << 2)
#define TASK_FLG_BUSY           (1 << 3)

void task_busy(void *task);
