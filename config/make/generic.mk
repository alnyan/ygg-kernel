CFLAGS=-ffreestanding -nostdlib -nostartfiles -Isrc
LDFLAGS=-nostdlib -nostartfiles
LDFLAGS_POST=

OBJS+=build/kernel.o \
	  build/sys/debug.o \
	  build/sys/mem.o \
	  build/sys/string.o \
	  build/sys/elf.o \
	  build/dev/initrd.o \
	  build/sys/panic.o \
	  build/dev/vfs.o \
	  build/sys/heap.o
HDRS+=src/util.h \
	  src/sys/elf.h \
	  src/dev/initrd.h \
	  src/sys/panic.h \
	  src/dev/vfs.h \
	  src/sys/heap.h

DIRS+=build/sys \
	  build/dev

