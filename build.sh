#!/bin/bash
set -e	# stop immediately if any command fails

i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -T linker.ld -o myos -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
grub-file --is-x86-multiboot myos && echo multiboot confirmed
cp myos isodir/boot/myos
grub-mkrescue -o myos.iso isodir
qemu-system-i386 -cdrom myos.iso -display curses
