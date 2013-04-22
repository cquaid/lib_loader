#!/bin/bash
echo "compiling library"
gcc -fpic -c lib.c
if [ $? -ne 0 ]; then
	echo "Failed: gcc  -O0 -fpic -c lib.c"
	exit 1
fi
gcc -shared -Wl,-soname,lib.so.1 -o lib.so.1.0 lib.o
if [ $? -ne 0 ]; then
	echo "Failed: gcc -shared -Wl,-soname,lib.so.1 -o lib.so.1.0 lib.o"
	exit 1
fi
mv lib.so.1.0 lib.so
cp lib.so llib.so
strip llib.so
rm -f lib.o

echo "compiling test_loader"
gcc -Wall -g test_loader.c ../bintree/bintree.c ../rtld/rtld.c ../list/list.c -o test_loader -DDEBUG
if [ $? -ne 0 ]; then
	echo "Failed: gcc test_loader"
	exit 1
fi
#cp a.out load
#echo "'cat'-ing library"
#cat out/llib.so >>load
