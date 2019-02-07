CFLAGS=-ffreestanding -nostdlib -nostartfiles -Isrc
LDFLAGS=-nostdlib -nostartfiles
LDFLAGS_POST=

OBJS+=build/kernel.o \
	  build/sys/debug.o

DIRS+=build/sys

