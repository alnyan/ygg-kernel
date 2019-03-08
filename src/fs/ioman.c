#include "ioman.h"
#include "sys/task.h"
#include "sys/debug.h"
#include <stddef.h>

static void ioman_task(void *arg) {
    kdebug("PRE SLEEP\n");
    asm volatile ("int $0x80"::"a"(0), "b"(1));
    kdebug("POST SLEEP\n");

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
