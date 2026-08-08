#!/bin/sh
set -e

for prog in hello hello2; do
	i686-eclipseos-gcc -O2 -Wall -Wextra -fPIE \
		-c user/$prog.c -o user/$prog.o
	i686-eclipseos-gcc -pie -o initrd/$prog.elf user/$prog.o
	echo "built initrd/$prog.elf"
done
