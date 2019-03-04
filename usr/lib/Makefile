libc_OBJS=$(O)/posix/printf.o \
		  $(O)/posix/string.o \
		  $(O)/posix/vsnprintf.o \
		  $(O)/posix/unistd.o \
		  $(O)/posix/signal.o \
		  $(O)/posix/abort.o \
		  $(O)/sys/k/syscall.o
libc_CFLAGS=-ffreestanding \
			-nostdlib \
			-Iinclude \
			-I../../include

CRTI=$(O)/crt/x86/crti.o
CRTN=$(O)/crt/x86/crtn.o
CRTBEGIN=$(shell $(CC) -print-file-name=crtbegin.o)
CRTEND=$(shell $(CC) -print-file-name=crtend.o)
CRT0=$(O)/crt/x86/crt0.o
CRTX_OBJS=$(CRTI) \
		  $(CRT0) \
		  $(CRTN)

DIRS=$(O)/crt/x86 \
	 $(O)/posix \
	 $(O)/sys/k

all: mkdirs $(CRTX_OBJS) $(O)/libusr.a

mkdirs:
	@mkdir -p $(DIRS)

#####

$(O)/crt/x86/%.o: crt/x86/%.S
	@printf " AS\t%s\n" $(<:%.S=%.o)
	@$(CC) -c -o $@ $<

#####

$(O)/libusr.a: $(libc_OBJS)
	@printf " AR\t%s\n" libc/libusr.a
	@$(AR) rcs $@ $(libc_OBJS)

$(O)/%.o: %.c
	@printf " CC\t%s\n" $(<:%.c=libc/%.o)
	@$(CC) $(libc_CFLAGS) -c -o $@ $<
