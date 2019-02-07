CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
OBJCOPY=$(CROSS_COMPILE)objcopy

CFLAGS=-ffreestanding -nostdlib -nostartfiles
LDFLAGS=-nostdlib -nostartfiles -Tsrc/linker.ld

OBJS=build/boot.o \
	 build/kernel.o \
	 build/arch/aarch64/board/bcm2837.o

all: mkdirs build/kernel.bin

mkdirs:
	mkdir -p build/arch/aarch64/board

clean:
	rm -rf build

build/%.o: src/%.s
	$(AS) -o $@ $<

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

build/kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary $< $@

build/kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
