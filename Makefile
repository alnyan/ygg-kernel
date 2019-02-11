CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
OBJCOPY=$(CROSS_COMPILE)objcopy

all: clean mkdirs build/kernel.bin

include config/make/generic.mk

ifeq ($(ARCH),)
$(error "No $${ARCH} set")
endif

ifneq ($(BOARD),)
include config/make/$(ARCH)/$(BOARD).mk
else
include config/make/$(ARCH)/generic.mk
endif

mkdirs:
	mkdir -p $(DIRS)

clean:
	rm -rf build

build/%.o: src/%.s
	$(AS) -o $@ $<

build/%.o: src/%.c
	$(CC) -ggdb $(CFLAGS) -c -o $@ $<

build/kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary $< $@

build/kernel.elf: $(BOOT_OBJS) $(OBJS)
	$(LD) $(LDFLAGS) -T$(LINKER) -o $@ $(BOOT_OBJS) $(OBJS) $(LDFLAGS_POST)
