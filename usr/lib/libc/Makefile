libc_OBJS=$(O)/posix/printf.o \
		  $(O)/posix/string.o \
		  $(O)/posix/vsnprintf.o \
		  $(O)/posix/unistd.o \
		  $(O)/posix/signal.o \
		  $(O)/posix/abort.o \
		  $(O)/posix/errno.o \
		  $(O)/sys/ygg/syscall.o
libc_CFLAGS=-ffreestanding \
			-nostdlib \
			-Iinclude \
			-I$(SRC)/include

DIRS=$(O)/posix \
	 $(O)/sys/ygg

all: mkdirs $(O)/libusr.a

mkdirs:
	@mkdir -p $(DIRS)

#####

$(O)/libusr.a: $(libc_OBJS)
	@printf " AR\t%s\n" libc/libusr.a
	@$(AR) rcs $@ $(libc_OBJS)

$(O)/%.o: %.c
	@printf " CC\t%s\n" $(<:%.c=libc/%.o)
	@$(CC) $(libc_CFLAGS) -c -o $@ $<
