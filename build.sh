#!/bin/sh
set -e
. ./headers.sh

for PROJECT in $PROJECTS; do
  (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done

# Second pass: embed the symbol table
./gensyms.sh sysroot/boot/eclipseos.kernel kernel/arch/i386/symbols.c
(cd kernel && DESTDIR="$SYSROOT" $MAKE install)
