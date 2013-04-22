#define _LARGEFILE64_SOURCE /* lseek64 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "zlib_list.h"
#include "lib_loader.h"

static Anchor alist[] = {
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

void
create_zlib_fixup(void)
{
	Anchor *beg;

	beg = alist;
	while (beg->name != NULL && beg->symbol != NULL)
		add_fixup_anchor(beg++);
}
