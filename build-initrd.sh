#!/bin/sh
set -e
./build-user.sh
cd initrd
tar --format=ustar -cf ../initrd.tar *
cd ..
echo "initrd.tar built"
