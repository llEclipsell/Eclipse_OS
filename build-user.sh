#!/bin/sh
set -e

SYSROOT="$(pwd)/sysroot"

i686-eclipseos-gcc --sysroot="$SYSROOT" -ffreestanding -nostdlib -fPIE -O2 \
	-c user/hello.c -o user/hello.o

i686-eclipseos-gcc --sysroot="$SYSROOT" -ffreestanding -nostdlib -fPIE \
	-c libc/arch/i386/syscall.S -o user/syscall.o

i686-eclipseos-ld -pie -e _start -o initrd/hello.elf user/hello.o user/syscall.o

echo "built initrd/hello.elf"
