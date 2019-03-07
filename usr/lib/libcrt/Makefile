
CRTI=$(O)/libcrt/x86/crti.o
CRTN=$(O)/libcrt/x86/crtn.o
CRTBEGIN=$(shell $(CC) -print-file-name=crtbegin.o)
CRTEND=$(shell $(CC) -print-file-name=crtend.o)
CRT0=$(O)/libcrt/x86/crt0.o
CRTX_OBJS=$(CRTI) \
		  $(CRT0) \
		  $(CRTN)

DIRS=$(O)/libcrt/x86

all: mkdirs $(CRTX_OBJS)

mkdirs:
	@mkdir -p $(DIRS)

$(O)/libcrt/x86/%.o: x86/%.S
	@$(CC) -c -o $@ $<
