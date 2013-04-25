#define _LARGEFILE64_SOURCE /* lseek64 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "librtld.h"
#include "zlib_stuff.h"

static Anchor zlib_anchors[] = {
#define a(x) { #x, x },	
	a(write)
	a(strlen)
	a(fprintf)
	a(printf)
	a(malloc)
	a(free)
	a(strcpy)
	a(memset)
	a(close)
	a(memchr)
	a(read)
	a(memcpy)
	a(open)
	a(strcat)
	a(lseek64)
	a(strerror)
	a(__sprintf_chk)
	a(__vsnprintf_chk)
#undef a
	{ NULL, NULL }
};

int
main(int argc, char *argv[])
{
	elf_object *elf;

	add_fixup_anchor_list(zlib_anchors);

	elf = elf_dlopen("/lib/x86_64-linux-gnu/libz.so.1.2.7");
	if (elf == NULL)
		return -1;

	deflateInit = (deflateInit_fn)elf_dlsym(elf, "deflateInit_");
	if (deflateInit == NULL)
		goto out;

	deflateEnd = (deflateEnd_fn)elf_dlsym(elf, "deflateEnd");
	if (deflateEnd == NULL)
		goto out;
	
	deflate = (deflate_fn)elf_dlsym(elf, "deflate");
	if (deflate == NULL)
		goto out;

	inflateInit = (inflateInit_fn)elf_dlsym(elf, "inflateInit_");
	if (inflateInit == NULL)
		goto out;

	inflateEnd = (inflateEnd_fn)elf_dlsym(elf, "inflateEnd");
	if (inflateEnd == NULL)
		goto out;

	inflate = (inflate_fn)elf_dlsym(elf, "inflate");
	if (inflate == NULL)
		goto out;

	if (argc == 1) {
		int ret;
		ret = def(stdin, stdout, -1);
		if (ret != 0)
			zerr(ret);
		return ret;
	}

	if (argc == 2 && !strcmp(argv[1], "-d")) {
		int ret = inf(stdin, stdout);
		if (ret != 0)
			zerr(ret);
		return ret;
	}

	fprintf(stderr, "usage: a.out [-d] <source> dest\n");
	
	elf_dlclose(elf);
	return 0;

out:
	elf_dlclose(elf);
	return -2;
}

