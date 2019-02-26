ifeq ($(ARCH),x86)
TOOLS+=$(O)/x86/mmap-gen $(O)/x86/find
DIRS+=$(O)/x86
endif

# Here may later come build-dependencies too
all: tools

tools: mkdirs $(TOOLS)

mkdirs:
	@mkdir -p $(DIRS)

$(O)/%: %.c
	@$(HOSTCC) -o $@ $<
	@printf " HOSTCC\tutil/%s\n" "$(<:%.c=%.o)"
