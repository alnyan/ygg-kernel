LINKER=config/ld/x86/generic.ld

BOOT_OBJS+=build/arch/x86/boot.o
OBJS+=build/arch/x86/hw.o \
	  build/arch/x86/com.o \
	  build/arch/x86/gdt.o \
	  build/arch/x86/ints.o

DIRS+=build/arch/x86

LD=$(CC)
CFLAGS+=-DARCH_X86
LDFLAGS+=-static-libgcc
LDFLAGS_POST=-lgcc

QEMU_BIN?=qemu-system-i386
QEMU_CMD?=$(QEMU_BIN) \
		  -serial mon:stdio \
		  -kernel build/kernel.elf \
		  -nographic $(QEMU_ADD)
ifdef QEMU_DEBUG
QEMU_CMD+= -s -S
endif

qemu: clean mkdirs build/kernel.bin
	$(QEMU_CMD)
qemu-iso: clean mkdirs build/kernel.iso
	$(QEMU_BIN) -serial stdio -cdrom build/kernel.iso

build/kernel.iso: clean mkdirs build/kernel.elf
	mkdir -p build/isotmp/boot/grub
	cp src/arch/x86/grub.cfg build/isotmp/boot/grub
	cp build/kernel.elf build/isotmp/boot/kernel
	grub-mkrescue build/isotmp -o build/kernel.iso
	rm -rf build/isotmp
