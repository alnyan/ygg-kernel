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
	  build/arch/x86/multiboot.o \
	  build/arch/x86/console.o \
	  build/arch/x86/syscall.o \
	  build/arch/x86/panic.o \
	  build/arch/x86/task_s.o

USR_LINKER=config/ld/x86/user.ld

DIRS+=build/arch/x86 \
	  build/usr

LD=$(CC)
CFLAGS+=-DARCH_X86
DEFS+=-DARCH_X86
LDFLAGS+=-static-libgcc
LDFLAGS_POST=-lgcc

UTILS+=x86/mmap-gen

QEMU_BIN?=qemu-system-i386
QEMU_CMD?=$(QEMU_BIN) \
		  -serial mon:stdio \
		  -kernel build/kernel.elf \
		  -initrd build/usr/init.tar $(QEMU_ADD)
ifdef QEMU_DEBUG
QEMU_CMD+= -s -S
endif

qemu: all
	@$(QEMU_CMD)

qemu-iso: iso
	@$(QEMU_BIN) -serial stdio -cdrom build/dist.iso

build/dist.iso: all
	@printf " ISO\t%s\n" $@
	@mkdir -p isotmp/boot/grub
	@cp src/arch/x86/grub.cfg isotmp/boot/grub
	@cp build/kernel.elf isotmp/boot/kernel
	@cp build/usr/init.tar isotmp/boot/init
	@grub-mkrescue -o $@ isotmp 2>/dev/null >/dev/null
	@rm -rf isotmp

iso: build/dist.iso
