CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
AR=$(CROSS_COMPILE)ar
HOSTCC=gcc
HOSTLD=ld
HOSTAS=as
OBJCOPY=$(CROSS_COMPILE)objcopy

O=$(abspath build)
export SRC=$(abspath .)

HDRS=$(shell find src -name "*.h")
CONFIG=$(shell find . -maxdepth 1 -name ".config")

ifneq ($(CONFIG),)
include $(CONFIG)
endif

all: pre_build mkdirs build/kernel.bin utils userspace post doc

dump:
	@echo Output directory: $(O)
	@echo "Build files:"
	@echo $(OBJS)

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

DOCDIRS+=build/doc/html
DOCHTML=$(DOCS:doc/%.rst=build/doc/html/%.html)

VERSION_GIT=$(shell git describe --tags --always)
DEFINES+=-DYGG_NAME="\"yggdrasil\"" \
		 -DYGG_VERSION="\"$(VERSION_GIT)\"" \
		 -DYGG_REVDATE="\"$(shell date --rfc-email)\""

ifneq ($(BOARD),)
DEFINES+=-DYGG_TARGET="\"$(ARCH)-$(BOARD)\""
else
DEFINES+=-DYGG_TARGET="\"$(ARCH)\""
endif

doc: doc-dirs doc-html

doc-dirs:
	@$(foreach d,$(DOCDIRS),mkdir -p "$d";)

doc-html: $(DOCHTML)

build/doc/html/%.html: doc/%.rst
	@printf " HTML\t%s\n" $<
	@rst2html $< > $@

pre_build:
	@$(foreach cmd,$(PRE_BUILD_HOOKS),$(cmd))

pre_user:
	@$(foreach cmd,$(PRE_USER_HOOKS),$(cmd))

post:
	@$(foreach cmd,$(POST_HOOKS),$(cmd))

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

userspace: pre_user
	@mkdir -p build/usr
	@AR=$(AR) CC=$(CC) LD=$(LD) O=$(O)/usr make -s -C usr all

utils:
	@mkdir -p build/util
	@HOSTCC=$(HOSTCC) HOSTLD=$(HOSTLD) HOSTAS=$(HOSTAS) ARCH=$(ARCH) O=$(O)/util make -s -C util all
