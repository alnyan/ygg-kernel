CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
AR=$(CROSS_COMPILE)ar
HOSTCC=gcc
HOSTLD=ld
HOSTAS=as
OBJCOPY=$(CROSS_COMPILE)objcopy

HDRS=$(shell find src -name "*.h")
CONFIG=$(shell find . -maxdepth 1 -name ".config")

ifneq ($(CONFIG),)
include $(CONFIG)
endif

all: mkdirs build/kernel.bin userspace utils

include config/make/generic.mk

ifeq ($(ARCH),)
$(error "No $${ARCH} set")
endif

ifneq ($(BOARD),)
include config/make/$(ARCH)/$(BOARD).mk
else
include config/make/$(ARCH)/generic.mk
endif

include config/make/driver.mk

mkdirs:
	@mkdir -p $(DIRS)

clean:
	@rm -rf build

build/%.o: src/%.s
	@$(error "$<: We don't do that here. Please rename it to .S")

build/%.o: src/%.S $(CONFIG)
	@printf " AS\t%s\n" "$<"
	@$(CC) -ggdb $(DEFINES) -Isrc -c -o $@ $<

build/%.o: src/%.c $(HDRS) $(CONFIG)
	@printf	" CC\t%s\n" "$<"
	@$(CC) -ggdb $(DEFINES) $(CFLAGS) -c -o $@ $<

build/kernel.bin: build/kernel.elf
	@printf " OC\t%s\n" "$@"
	@$(OBJCOPY) -O binary $< $@

build/kernel.elf: $(BOOT_OBJS) $(OBJS)
	@printf " LD\t%s\n" "$@"
	@$(LD) $(LDFLAGS) -T$(LINKER) -o $@ $(BOOT_OBJS) $(OBJS) $(LDFLAGS_POST)

userspace:
	@mkdir -p build/usr
	@AR=$(AR) CC=$(CC) LD=$(LD) O=../build/usr make -s -C usr all

utils:
	@mkdir -p build/util
	@HOSTCC=$(HOSTCC) HOSTLD=$(HOSTLD) HOSTAS=$(HOSTAS) ARCH=$(ARCH) O=../build/util make -s -C util all
