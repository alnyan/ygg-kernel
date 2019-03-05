************************
Tasks and scheduling API
************************

0. Overview and constants
#########################


1. Task object manipulation
###########################

``task_t *task_create(void)``
   Creates an uninitialized platform-specific task object.

   Returned values:
      * Task object pointer in case of success
      * NULL if allocation fails

``void task_destroy(task_t *t)``
   Destroys the task and de-allocates the resources used by it, including:
      * Virtual memory spaces
      * Context info structures
      * Physical memory
      * File descriptors
      * etc.

``uint32_t task_flags(task_t *t)``
   Returns bitfield of task's flags.

``task_ctl_t *task_ctl(task_t *)``
   Returns general task control structure.


2. Signals
##########

``void task_signal(task_t *t, int signum)``
   Notifies the task about some external event, activating the signal `signum`.

``int task_issig(task_t *t)``
   Returns non-zero values if the task is currently handling a signal.

`TODO: sigaltstack?`


3. Waits
########

``void task_sleep_deadline(task_t *t, uint64_t systick)``
   Sets the task's sleep deadline value to `systick`, which means that once the system timer value
   reaches `systick`, the task will be woken up, unless the sleep is aborted by a signal. If the
   task already has a deadline set, overrides it.

``void task_waitpid(task_t *t, int pid)``
   Sets the task to wait for completion of process `pid`, unless interrupted by a signal.

`TODO: pending I/O settings`

``int task_iswait(task_t *t)``
   Returns non-zero values if the task is in the waiting state.


4. I/O
######

``vfs_file_t *task_get_fd(task_t *t, int fd)``
   Returns the value of VFS file descriptor associated with numeric descriptor `fd`, if the latter
   is a valid file descriptor.

``int task_bind_fd(task_t *t, vfs_file_t *f)``
   Attaches a VFS file descriptor to the task object's descriptor table, if possible.

   Returned values:
      * >= 0 numeric descriptor of the "slot" the VFS descriptor is attached to
      * `-EMFILE` if the table has hit its limit

``void task_ubind_fd(task_t *t, int fd)``
   Detaches a VFS file described by `fd` numeric handle, if the latter is a valid file descriptor.
   For actually closing a VFS file, use `vfs_close()`

``vfs_file_t *task_pending_io(task_t *t)``
   Returns a VFS descriptor which has pending I/O job running, if present. If no job is running,
   NULL is returned.


5. Memory
#########

``mm_space_t task_space(task_t *t)``
   Returns the pointer to the task's memory space structure.

``uintptr_t task_getend(task_t *t)``
   Returns the actual virtual address of the end of task's image data in memory.

``uintptr_t task_getbrk(task_t *t)``
   Returns the break value for the task (which is most likely used as an end address for heap in
   userspace, starting with the `task_getend()` value, i.e., located right after task's image data).

``int task_brk(task_t *t, uintptr_t v)``
   Sets the break value for the task to `v`, if the latter is a reasonable value and the system has
   enough memory. If the value is smaller than the previous break value, may free the allocated
   memory pages.

   Returned values:
      * 0 in case of success
      * `-ENOMEM` in case the system doesn't have enough memory
      * `TODO: return value if the value is smaller than task_getend()`

``int task_sbrk(task_t *t, size_t v)``
   Increases the break value for the task by `v` bytes, optionally aligning the values to some
   platform-specific boundaries.

   Returned values are the same as in `task_brk()` calls

6. Scheduler
############

``task_t *task_sched(void)``
   Selects a new `task_current` and returns the value of previous task pointer.

``pid_t task_add(task_t *t)``
   Adds a new task to the scheduler queue, assigning it a new PID value, which is the returned.
