bin_PROGRAMS=$(O)/bin/init \
		  $(O)/bin/hello \
		  $(O)/bin/ls \
		  $(O)/bin/cpuid
bin_CFLAGS=-I ../include \
		   -nostdlib \
		   -ffreestanding \
		   -I lib/libc/include
bin_LDFLAGS=-T ../config/ld/x86/user.ld \
			-nostdlib

etc_CONFIGS=$(O)/etc/network.conf

CRTI=$(O)/lib/libcrt/x86/crti.o
CRTN=$(O)/lib/libcrt/x86/crtn.o
CRTBEGIN=$(shell $(CC) -print-file-name=crtbegin.o)
CRTEND=$(shell $(CC) -print-file-name=crtend.o)
CRT0=$(O)/lib/libcrt/x86/crt0.o
CRTX_OBJS=$(CRTI) \
		  $(CRT0) \
		  $(CRTN)
LIBC_STATIC=$(O)/lib/libusr.a
LIBC_HEADERS=$(shell find lib -name "*.h")

DIRS=$(O)/bin \
	 $(O)/etc

all: mkdirs libs $(O)/init.tar

mkdirs:
	mkdir -p $(DIRS)

libs:
	@make -s -C lib all O=$(O)/lib CC=$(CC) AS=$(AS)

$(O)/init.tar: $(bin_PROGRAMS) $(etc_CONFIGS)
	@printf " ROOTFS\t%s\n" "init.tar"
	@tar -C $(O) -cf $@ bin etc

$(O)/etc/%: etc/%
	@printf " CONF\t%s\n" "/$<"
	@cp "$<" "$@"

$(O)/bin/%: bin/%.c $(LIBC_STATIC) $(LIBC_HEADERS)
	@printf " CC\t%s\n" "/$(<:%.c=%)"
	@$(CC) $(bin_CFLAGS) $(bin_LDFLAGS) -o $@ $(CRTI) $(CRTBEGIN) $(CRT0) $< $(LIBC_STATIC) $(CRTEND) $(CRTN) -lgcc

$(O)/bin/%: bin/%.S $(LIBC_STATIC) $(LIBC_HEADERS)
	@printf " AS\t%s\n" "/$(<:%.S=%)"
	@$(CC) $(bin_CFLAGS) $(bin_LDFLAGS) -o $@ $(CRTI) $(CRTBEGIN) $(CRT0) $< $(LIBC_STATIC) $(CRTEND) $(CRTN) -lgcc
