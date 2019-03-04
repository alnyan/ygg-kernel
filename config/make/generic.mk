ifneq ($(ENABLE_MAP_TRACE),)
DEFINES+=-DENABLE_MAP_TRACE
endif

ifneq ($(ENABLE_HEAP_TRACE),)
DEFINES+=-DENABLE_HEAP_TRACE
endif

ifneq ($(ENABLE_SCHED_TRACE),)
DEFINES+=-DENABLE_SCHED_TRACE
endif

ifeq ($(DISABLE_TASK),)
DEFINES+=-DENABLE_TASK
OBJS+=build/sys/task.o \
 	  build/sys/elf.o
endif

ifeq ($(DISABLE_HEAP_ALLOC_COUNT),)
DEFINES+=-DENABLE_HEAP_ALLOC_COUNT
endif

CFLAGS=-ffreestanding \
	   -nostdlib \
	   -nostartfiles \
	   -Isrc \
	   -Wall \
	   -Wpedantic \
	   -Werror \
	   -Wno-unused-function \
	   -Wno-unused-const-variable \
	   -Wno-unused-but-set-variable \
	   -Wno-overlength-strings
LDFLAGS=-nostdlib -nostartfiles
LDFLAGS_POST=

OBJS+=build/kernel.o \
	 build/sys/string.o \
	 build/sys/ctype.o \
	 build/sys/debug.o \
	 build/sys/mem.o \
	 build/sys/panic.o \
	 build/sys/time.o \
	 build/sys/vfs.o \
	 build/sys/heap.o \
	 build/dev/devfs.o \
     build/dev/pseudo.o \
	 build/dev/initrd.o \
 	 build/sys/dev.o \
 	 build/sys/atoi.o \
	 build/net/eth/eth.o \
	 build/net/arp.o \
	 build/net/inet.o \
	 build/dev/net.o \
	 build/dev/tty.o

DIRS+=build/sys \
	  build/dev \
	  build/net/eth

