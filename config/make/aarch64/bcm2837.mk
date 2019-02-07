include config/make/aarch64/generic.mk

OBJS+=build/arch/aarch64/board/bcm2837/board.o \
	  build/arch/aarch64/board/bcm2837/uart.o \
	  build/arch/aarch64/board/bcm2837/gpio.o
DIRS+=build/arch/aarch64/board/bcm2837

CFLAGS+=-DBOARD_BCM2837

all:

info-board:
	@echo "Target board is BCM2837"

qemu: clean mkdirs build/kernel.bin
	qemu-system-aarch64 \
		-machine raspi3 \
		-kernel build/kernel.bin \
		-serial stdio
