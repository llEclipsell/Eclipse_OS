#!/bin/sh
set -e

i686-elf-gcc -ffreestanding -nostdlib -fPIE -O2 -Wall -Wextra \
	-c user/hello.c -o user/hello.o

i686-elf-ld -pie -e _start -o initrd/hello.elf user/hello.o

echo "built initrd/hello.elf (PIE)"
