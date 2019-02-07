
LINKER=config/ld/x86/generic.ld

BOOT_OBJS+=build/arch/x86/boot.o
OBJS+=build/arch/x86/hw.o \
	  build/arch/x86/com.o

DIRS+=build/arch/x86

LD=$(CC)
CFLAGS+=-DARCH_X86
LDFLAGS+=-static-libgcc
LDFLAGS_POST=-lgcc

qemu: clean mkdirs build/kernel.bin
	qemu-system-i386 \
		-kernel build/kernel.elf \
		-serial stdio
