
CRTI=$(O)/libcrt/$(ARCH)/crti.o
CRTN=$(O)/libcrt/$(ARCH)/crtn.o
CRTBEGIN=$(shell $(CC) -print-file-name=crtbegin.o)
CRTEND=$(shell $(CC) -print-file-name=crtend.o)
CRT0=$(O)/libcrt/$(ARCH)/crt0.o
CRTX_OBJS=$(CRTI) \
		  $(CRT0) \
		  $(CRTN)

DIRS=$(O)/libcrt/$(ARCH)

all: mkdirs $(CRTX_OBJS)

mkdirs:
	@mkdir -p $(DIRS)

$(O)/libcrt/$(ARCH)/%.o: $(ARCH)/%.S
	@printf " AS\t%s\n" $(<:%.S=libcrt/%.o)
	@$(CC) -c -o $@ $<
