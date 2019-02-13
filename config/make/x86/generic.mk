LINKER=config/ld/x86/generic.ld

BOOT_OBJS+=build/arch/x86/boot.o
OBJS+=build/arch/x86/hw.o \
	  build/arch/x86/com.o \
	  build/arch/x86/gdt.o \
	  build/arch/x86/ints.o \
	  build/arch/x86/ints_s.o \
	  build/arch/x86/pic8259.o \
	  build/arch/x86/timer.o \
	  build/arch/x86/mm.o \
	  build/arch/x86/task.o \
	  build/arch/x86/irq0.o \
	  build/arch/x86/regs.o \
	  build/arch/x86/ps2.o \
	  build/arch/x86/irqs_s.o \
	  build/arch/x86/multiboot.o

DIRS+=build/arch/x86

LD=$(CC)
CFLAGS+=-DARCH_X86
LDFLAGS+=-static-libgcc
LDFLAGS_POST=-lgcc

UTILS+=x86/mmap-gen

QEMU_BIN?=qemu-system-i386
QEMU_CMD?=$(QEMU_BIN) \
		  -serial mon:stdio \
		  -kernel build/kernel.elf $(QEMU_ADD)
ifdef QEMU_DEBUG
QEMU_CMD+= -s -S
endif

qemu: clean mkdirs build/kernel.bin
	$(QEMU_CMD)

qemu-iso: clean mkdirs build/kernel.iso
	$(QEMU_BIN) -serial stdio -cdrom build/kernel.iso

build/kernel.iso: clean mkdirs build/kernel.elf
	mkdir -p isotmp/boot/grub
	cp src/arch/x86/grub.cfg isotmp/boot/grub
	cp build/kernel.elf isotmp/boot/kernel
	grub-mkrescue -o build/kernel.iso isotmp
	rm -rf isotmp
