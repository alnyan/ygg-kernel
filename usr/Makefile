LIBUSR_OBJS=$(O)/lib/syscall.o

DIRS=$(O)/lib

all: mkdirs $(O)/init.elf

mkdirs:
	mkdir -p $(DIRS)

$(O)/init.elf: $(O)/init.o $(O)/libusr.a
	$(LD) -nostdlib -o $@ $(O)/init.o $(O)/libusr.a

$(O)/init.o: init.c
	$(CC) -ggdb -nostdlib -ffreestanding -c -o $@ $<

$(O)/libusr.a: $(LIBUSR_OBJS)
	$(AR) rcs $@ $(LIBUSR_OBJS)

$(O)/lib/%.o: lib/%.c
	$(CC) -nostdlib -c -ffreestanding -o $@ $<

