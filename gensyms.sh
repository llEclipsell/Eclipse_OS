#!/bin/sh
set -e

echo "#include <kernel/debug.h>"          >  "$2"
echo "const struct ksymbol ksymbols[] = {" >> "$2"

i686-elf-nm -n "$1" | grep -i ' [tT] ' | \
	awk '{ printf "\t{0x%s, \"%s\"},\n", $1, $3 }' >> "$2"

echo "\t{0, 0}\n};" >> "$2"
