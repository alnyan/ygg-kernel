LIBUSR_OBJS=$(O)/lib/printf.o \
			$(O)/lib/unistd.o \
			$(O)/lib/string.o
USR_PROGS=$(O)/bin/init

DIRS=$(O)/lib \
	 $(O)/bin

all: mkdirs $(O)/lib/libusr.a $(O)/init.tar

mkdirs:
	mkdir -p $(DIRS)

$(O)/init.tar: $(USR_PROGS)
	tar -C $(O) -cf $@ bin

$(O)/bin/%: bin/%.c
	$(CC) -Ilib -nostdlib -ffreestanding -o $@ $< $(O)/lib/libusr.a -lgcc

$(O)/lib/libusr.a: $(LIBUSR_OBJS)
	$(AR) rcs $@ $(LIBUSR_OBJS)

$(O)/lib/%.o: lib/%.c
	$(CC) -nostdlib -c -ffreestanding -o $@ $<

