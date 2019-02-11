
LINKER=config/ld/x86/generic.ld

BOOT_OBJS+=build/arch/x86/boot.o
OBJS+=build/arch/x86/hw.o \
	  build/arch/x86/com.o

DIRS+=build/arch/x86

LD=$(CC)
CFLAGS+=-DARCH_X86
LDFLAGS+=-static-libgcc
LDFLAGS_POST=-lgcc

QEMU_BIN?=qemu-system-i386
QEMU_CMD?=$(QEMU_BIN) \
		  -kernel build/kernel.elf \
		  -serial mon:stdio \
		  -nographic $(QEMU_ADD)
ifdef QEMU_DEBUG
QEMU_CMD+= -s -S
endif

qemu: clean mkdirs build/kernel.bin
	$(QEMU_CMD)
