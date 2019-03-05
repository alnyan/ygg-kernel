LINKER=config/ld/x86/generic.ld

BOOT_OBJS+=build/arch/x86/boot.o
OBJS+=build/arch/x86/multiboot.o \
	  build/arch/x86/mm/phys.o \
	  build/arch/x86/mm/space.o \
	  build/arch/x86/mm/map.o \
	  build/arch/x86/mm/mm.o \
	  build/arch/x86/hw/com.o \
	  build/arch/x86/panic.o \
	  build/arch/x86/regs.o \
	  build/arch/x86/hw/console.o \
	  build/arch/x86/hw/hw.o \
	  build/arch/x86/hw/cpuid_s.o \
	  build/arch/x86/hw/cpuid.o \
	  build/arch/x86/hw/gdt.o \
	  build/arch/x86/hw/acpi.o \
	  build/arch/x86/hw/hpet.o \
	  build/arch/x86/hw/ints.o \
	  build/arch/x86/hw/ints_s.o \
	  build/arch/x86/hw/pic8259.o \
	  build/arch/x86/hw/irqs_s.o \
	  build/arch/x86/hw/irq0.o \
	  build/arch/x86/hw/periph_irq.o \
	  build/arch/x86/hw/rtc.o \
	  build/arch/x86/hw/ps2.o \
	  build/arch/x86/hw/timer.o \
	  build/arch/x86/hw/pfault_s.o \
	  build/arch/x86/hw/pfault.o \
	  build/arch/x86/hw/pci.o \
	  build/arch/x86/hw/fpu.o \
	  build/arch/x86/hw/fpu_s.o

ifeq ($(DISABLE_TASK),)
OBJS+=build/arch/x86/task/task.o \
	  build/arch/x86/task/task_s.o \
	  build/arch/x86/task/fork.o \
	  build/arch/x86/syscall.o \
	  build/arch/x86/mm/user.o
endif

ifneq ($(ENABLE_VESA_FBCON),)
OBJS+=build/arch/x86/hw/vesa/font8x8.o
DEFINES+=-DENABLE_VESA_FBCON
endif

ifneq ($(ENABLE_KERNEL_MAP),)
PRE_USER_HOOKS+=mkdir -p ./build/usr/etc && ./build/util/x86/mmap-gen ./build/kernel.elf > ./build/usr/etc/kernel.map;
DEFINES+=-DENABLE_KERNEL_MAP
endif

USR_LINKER=config/ld/x86/user.ld

DIRS+=build/arch/x86/hw \
	  build/arch/x86/task \
	  build/arch/x86/hw/vesa \
	  build/arch/x86/mm

LD=$(CC)
CFLAGS+=-DARCH_X86 -msse
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

ifneq ($(ENABLE_RTL8139),)
QEMU_ADD+= -device rtl8139,netdev=net0 \
		   -netdev user,id=net0,hostfwd=tcp::5432-:22,net=192.168.123.1/24 \
		   -object filter-dump,id=f1,netdev=net0,file=dump.dat
endif

qemu: all
	@$(QEMU_CMD)

qemu-iso: iso
	@$(QEMU_BIN) -serial stdio -cdrom build/dist.iso $(QEMU_ADD)

build/dist.iso: all
	@printf " ISO\t%s\n" $@
	@mkdir -p isotmp/boot/grub
	@cp src/arch/x86/grub.cfg isotmp/boot/grub
	@cp build/kernel.elf isotmp/boot/kernel
	@cp build/usr/init.tar isotmp/boot/init
	@grub-mkrescue -o $@ isotmp 2>/dev/null >/dev/null
	@rm -rf isotmp

iso: build/dist.iso
