include config/make/aarch64/generic.mk

OBJS+=build/arch/aarch64/board/bcm2837/board.o \
	  build/arch/aarch64/board/bcm2837/uart.o \
	  build/arch/aarch64/board/bcm2837/gpio.o \
	  build/arch/aarch64/board/bcm2837/irq.o
DIRS+=build/arch/aarch64/board/bcm2837

CFLAGS+=-DBOARD_BCM2837
QEMU_BIN?=qemu-system-aarch64
QEMU_CMD?=$(QEMU_BIN) \
		  -machine raspi3 \
		  -kernel build/kernel.bin \
		  -serial mon:stdio \
		  -nographic \
		  -d int

ifdef QEMU_DEBUG
QEMU_CMD+= -s -S
endif


all:

info-board:
	@echo "Target board is BCM2837"

qemu: clean mkdirs build/kernel.bin
	$(QEMU_CMD)
