CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
HOSTCC=gcc
HOSTLD=ld
HOSTAS=as
OBJCOPY=$(CROSS_COMPILE)objcopy

all: clean mkdirs build/kernel.bin
	HOSTCC=$(HOSTCC) make -C util $(UTILS)

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
	@$(AS) -o $@ $<
	@printf " AS\t%s\n" "$<"

build/%.o: src/%.c
	@$(CC) -ggdb $(CFLAGS) -c -o $@ $<
	@printf	" CC\t%s\n" "$<"

build/kernel.bin: build/kernel.elf
	@$(OBJCOPY) -O binary $< $@
	@printf " OC\t%s\n" "$@"

build/kernel.elf: $(BOOT_OBJS) $(OBJS)
	@$(LD) $(LDFLAGS) -T$(LINKER) -o $@ $(BOOT_OBJS) $(OBJS) $(LDFLAGS_POST)
	@printf " LD\t%s\n" "$@"
