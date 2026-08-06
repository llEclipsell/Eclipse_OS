#!/bin/sh
set -e

i686-elf-gcc -ffreestanding -nostdlib -O2 -Wall -Wextra \
	-c user/hello.c -o user/hello.o

i686-elf-gcc -T user/user.ld -ffreestanding -nostdlib \
	-o initrd/hello.elf user/hello.o -lgcc

echo "built initrd/hello.elf"
