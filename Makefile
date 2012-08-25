CC=gcc
LD=ld
DD=dd
OBJCOPY=objcopy

INCLUDEDIRS += -I$(shell pwd)/include -I.
CFLAGS = -Wall -O0 -m32 -nostdinc -nostdlib -ffreestanding -fno-stack-protector $(INCLUDEDIRS)

BOOTOBJS = boot/boot.o \
	boot/scr.o \
	boot/ide.o \
	boot/memory.o \
	boot/io.o \
	boot/fat16.o
BOOTSRCS = $(BOOTOBJS:.o=.c)

KERNOBJS = main.o \
	gdt.o \
	lock.o \
	pio.o \
	idt.o \
	isr.o \
	exception.o \
	timer.o \
	mem.o \
	paging.o \
	util.o \
	task.o \
	tss.o \
	device.o \
	drivers/tty.o \
	drivers/kbd.o \
	drivers/ide.o \
	vfs.o \
	fs/fat16.o \
	kio.o \
	syscall.o \
	apps/init.o
KERNSRCS = $(KERNOBJS:.o=.c)

USEROBJS = lib/syscall.o \
	lib/api.o \
	lib/io.o \
	lib/lib.o
USERSRCS = $(USEROBJS:.o=.c)

SNAKEOBJS = apps/snake.o
SNAKESRCS = $(SNAKEOBJS:.o=.c)

LIBGCC = $(shell $(CC) -m32 -print-libgcc-file-name)

debug: all
	qemu-system-i386 -s -boot c -hda fat16.dsc
	
run: all
	qemu-system-i386 -boot c -hda fat16.dsc

all : mbr fat16_disc
	$(DD) if=mbr.bin of=fat16.dsc bs=1 skip=0 seek=62 conv=notrunc

mbr: mbr.asm
	nasm -f bin -o mbr.bin -l mbr.lst mbr.asm

.c.o:
	gcc -c $(CFLAGS) $< -o $@

bootl: $(BOOTOBJS) boot/boot.asm
	nasm -f bin -o $(shell pwd)/boot_asm boot/boot.asm
	$(LD) -o boot_c_temp -Ttext 0x20200 -e main $(BOOTOBJS)
	$(OBJCOPY) -R .note -R .comment -S -O binary boot_c_temp boot_c
	cat boot_asm boot_c > boot.bin
	rm boot_c_temp boot_c boot_asm

kernel: $(KERNOBJS)
	$(LD) -o kernel_temp -Ttext 0x500000 -e kernel_main $(KERNOBJS) $(LIBGCC)
	$(OBJCOPY) -R .note -R .comment -S -O binary kernel_temp kernel.bin
	rm kernel_temp

snake: $(SNAKEOBJS) $(USEROBJS)
	$(LD) -o snake_temp -Ttext 0x40001000 -e crt_main $(SNAKEOBJS) $(USEROBJS) $(LIBGCC)
	objdump -D snake_temp > apps/snake.dmp
	$(OBJCOPY) -R .note -R .comment -S -O binary snake_temp snake.bin
	rm snake_temp

fat16_disc: bootl kernel snake
	dd if=/dev/zero of=fat16.dsc bs=512 count=40960
	mkfs.vfat -F 16 fat16.dsc
	mkdir disc
	mount -o loop fat16.dsc disc
	cp boot.bin disc/boot
	cp kernel.bin disc/kernel
	mkdir disc/apps
	cp snake.bin disc/apps/
	umount -l disc
	rmdir disc

clean:
	if [ -e "disc" ]; then umount -l disc ; rmdir disc ; fi
	rm -rf *.lst
	rm -rf *.o
	rm -rf *.bin
	rm -f fat16.dsc
	rm .depend
	touch .depend

dep:
	gcc -M $(INCLUDEDIRS) $(BOOTSRCS) > .depend
	gcc -M $(INCLUDEDIRS) $(KERNSRCS) >> .depend
	gcc -M $(INCLUDEDIRS) $(USERSRCS) >> .depend
	gcc -M $(INCLUDEDIRS) $(SNAKESRCS) >> .depend

include .depend
