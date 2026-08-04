#!/bin/sh
set -e
cd initrd
tar --format=ustar -cf ../initrd.tar *
cd ..
echo "initrd.tar built"
