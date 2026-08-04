#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp $SYSROOT/boot/myos.kernel isodir/boot/myos.kernel

./build-initrd.sh
cp initrd.tar isodir/boot/initrd.tar

cat > isodir/boot/grub/grub.cfg << EOF
menuentry "myos" {
	multiboot /boot/myos.kernel
	module /boot/initrd.tar
}
EOF
grub-mkrescue -o myos.iso isodir
