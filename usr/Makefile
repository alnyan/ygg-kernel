LIBUSR_OBJS=$(O)/lib/printf.o \
			$(O)/lib/unistd.o \
			$(O)/lib/string.o

DIRS=$(O)/lib

all: mkdirs $(O)/init.elf

mkdirs:
	mkdir -p $(DIRS)

$(O)/init.elf: $(O)/init.o $(O)/libusr.a
	$(LD) -static-libgcc -nostdlib -o $@ $(O)/init.o $(O)/libusr.a -lgcc

$(O)/init.o: init.c
	$(CC) -Ilib -ggdb -nostdlib -ffreestanding -c -o $@ $<

$(O)/libusr.a: $(LIBUSR_OBJS)
	$(AR) rcs $@ $(LIBUSR_OBJS)

$(O)/lib/%.o: lib/%.c
	$(CC) -nostdlib -c -ffreestanding -o $@ $<

