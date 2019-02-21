CFLAGS=-ffreestanding \
	   -nostdlib \
	   -nostartfiles \
	   -Isrc \
	   -Wall \
	   -Wpedantic \
	   -Werror \
	   -Wno-unused-function \
	   -Wno-unused-const-variable \
	   -Wno-unused-but-set-variable
LDFLAGS=-nostdlib -nostartfiles
LDFLAGS_POST=

OBJS+=build/kernel.o \
	  build/sys/debug.o \
	  build/sys/mem.o \
	  build/sys/string.o \
	  build/sys/elf.o \
	  build/dev/initrd.o \
	  build/sys/panic.o \
	  build/sys/heap.o \
	  build/sys/ctype.o \
	  build/sys/vfs.o \
	  build/dev/devfs.o \
	  build/dev/tty.o \
	  build/sys/dev.o \
	  build/sys/task.o \
	  build/dev/pseudo.o

DIRS+=build/sys \
	  build/dev

