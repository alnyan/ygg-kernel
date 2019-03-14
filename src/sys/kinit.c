#include "kinit.h"
#include "sys/task.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "fs/tarfs.h"
#include "../util.h"
#include "sys/heap.h"

void kinit_task(void) {
    kinfo("kinit started\n");

    assert(vfs_mount_path("/", NULL, vfs_tarfs, 0));

    task_fexecve("/bin/init", NULL, NULL);

    // After running an userspace init, act as [idle] process
    while (1) {
        for (int i = 0; i < 100000000; ++i);

        __idle();
    }
}