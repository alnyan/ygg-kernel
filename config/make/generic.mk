CFLAGS=-ffreestanding -nostdlib -nostartfiles -Isrc
LDFLAGS=-nostdlib -nostartfiles

OBJS+=build/kernel.o \
	  build/sys/debug.o

DIRS+=build/sys

