#!/bin/sh
set -e
. ./iso.sh

qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso -display curses -no-reboot -no-shutdown \
 -serial file:debug.log
