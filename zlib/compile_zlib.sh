#!/bin/bash
echo "compiling libz_loader"
gcc -Wall -g libz_loader.c ../bintree/bintree.c ../rtld/rtld.c ../list/list.c zlib_list.c -o libz_loader -DDEBUG
if [ $? -ne 0 ]; then
	echo "Failed: gcc libz_loader"
	exit 1
fi

