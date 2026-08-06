#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp $SYSROOT/boot/eclipseos.kernel isodir/boot/eclipseos.kernel

./build-initrd.sh
cp initrd.tar isodir/boot/initrd.tar

cat > isodir/boot/grub/grub.cfg << EOF
menuentry "eclipseos" {
	multiboot /boot/eclipseos.kernel
	module /boot/initrd.tar
}
EOF
grub-mkrescue -o eclipseos.iso isodir
