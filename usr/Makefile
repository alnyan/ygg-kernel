LIBUSR_OBJS=$(O)/lib/printf.o \
			$(O)/lib/unistd.o \
			$(O)/lib/string.o \
			$(O)/lib/vsnprintf.o
LIBUSR_HDRS=$(shell find lib -name "*.h")
USR_PROGS=$(O)/bin/init \
		  $(O)/bin/hello \
		  $(O)/bin/ls
CONFIGS=$(O)/etc/network.conf

LIBUSR_CFLAGS=-I ../include \
			  -nostdlib \
			  -ffreestanding \
			  -ggdb

USR_CFLAGS=-I ../include \
		   -nostdlib \
		   -ffreestanding \
		   -I lib
USR_LDFLAGS=-T ../config/ld/x86/user.ld

CRTI=$(O)/crtx/crti.o
CRTN=$(O)/crtx/crtn.o
CRTBEGIN=$(shell $(CC) -print-file-name=crtbegin.o)
CRTEND=$(shell $(CC) -print-file-name=crtend.o)
CRT0=$(O)/crtx/crt0.o
CRTX_OBJS=$(CRTI) \
		  $(CRT0) \
		  $(CRTN)

DIRS=$(O)/lib \
	 $(O)/bin \
	 $(O)/crtx \
	 $(O)/etc

all: mkdirs $(O)/lib/libusr.a $(CRTX_OBJS) $(O)/init.tar

mkdirs:
	mkdir -p $(DIRS)

$(O)/init.tar: $(USR_PROGS) $(CONFIGS)
	@printf " ROOTFS\t%s\n" "init.tar"
	@tar -C $(O) -cf $@ bin etc

$(O)/etc/%: etc/%
	@printf " CONF\t%s\n" "/$<"
	@cp "$<" "$@"

$(O)/bin/%: bin/%.c $(O)/lib/libusr.a
	@printf " CC\t%s\n" "/$(<:%.c=%)"
	@$(CC) $(USR_CFLAGS) $(USR_LDFLAGS) -o $@ $(CRTI) $(CRTBEGIN) $(CRT0) $< $(O)/lib/libusr.a $(CRTEND) $(CRTN) -lgcc

$(O)/lib/libusr.a: $(LIBUSR_OBJS)
	@printf " AR\t%s\n" "usr/lib/libusr.a"
	@$(AR) rcs $@ $(LIBUSR_OBJS)

$(O)/lib/%.o: lib/%.c $(LIBUSR_HDRS)
	@printf " CC\t%s\n" "usr/$(<:%.c=%.o)"
	@$(CC) $(LIBUSR_CFLAGS) -c -o $@ $<

$(O)/crtx/%.o: crtx/%.S
	@printf " AS\t%s\n" "usr/$(<:%.c=%.o)"
	@$(CC) -c -o $@ $<
