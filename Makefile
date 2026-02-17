ISO := build/osp.iso
KERNEL_ELF := build/kernel.elf
OBJS := build/boot64.o build/kernel.o build/terminal.o build/serial.o build/keyboard.o build/shell.o build/mmu.o build/sched.o build/proc.o

CFLAGS := -std=gnu11 -ffreestanding -fno-stack-protector -fno-pic -m64 -mno-red-zone -O2 -Wall -Wextra -Iinclude
LDFLAGS := -nostdlib -z max-page-size=0x1000 -T linker64.ld

.PHONY: all clean run test

all: $(ISO)

build:
	mkdir -p build iso/boot/grub

build/boot64.o: boot/boot64.asm | build
	nasm -f elf64 $< -o $@

build/%.o: kernel/%.c | build
	gcc $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJS) linker64.ld
	ld $(LDFLAGS) -o $@ $(OBJS)

iso/boot/kernel.elf: $(KERNEL_ELF) | build
	cp $(KERNEL_ELF) $@

iso/boot/grub/grub.cfg: grub/grub.cfg | build
	cp grub/grub.cfg iso/boot/grub/grub.cfg

$(ISO): iso/boot/kernel.elf iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) iso >/dev/null 2>&1

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -serial stdio -display none -no-reboot

test: $(ISO)
	timeout 8s qemu-system-x86_64 -cdrom $(ISO) -serial stdio -display none -no-reboot > build/qemu.log 2>&1 || true
	grep -q "OSP x86_64" build/qemu.log
	grep -q "MMU: 4-level paging enabled" build/qemu.log

clean:
	rm -rf build iso
