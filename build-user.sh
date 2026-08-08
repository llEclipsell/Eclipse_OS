#!/bin/sh
set -e

i686-eclipseos-gcc -O2 -Wall -Wextra -fPIE \
	-c user/hello.c -o user/hello.o

i686-eclipseos-gcc -pie -o initrd/hello.elf user/hello.o

echo "built initrd/hello.elf"
