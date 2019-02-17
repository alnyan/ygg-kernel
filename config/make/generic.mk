CFLAGS=-ffreestanding \
	   -nostdlib \
	   -nostartfiles \
	   -Isrc \
	   -Wall \
	   -Wpedantic \
	   -Werror \
	   -Wno-unused-function \
	   -Wno-unused-const-variable
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
	  build/sys/ctype.o
HDRS+=src/util.h \
	  src/sys/elf.h \
	  src/dev/initrd.h \
	  src/sys/panic.h \
	  src/sys/heap.h \
	  src/sys/ctype.h

DIRS+=build/sys \
	  build/dev

