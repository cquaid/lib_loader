#define _LARGEFILE64_SOURCE /* lseek64 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "zlib_list.h"
#include "../list/list.h"
#include "../rtld/anchor.h"

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

List*
create_zlib_list(void)
{
	List *ret;
	ListNode *node;
	Anchor *a;
	Anchor *beg;

	ret = ll_new_list();
	if (ret == NULL)
		return NULL;

#define add(al) do { \
					a = (Anchor *)malloc(sizeof(Anchor)); \
					if (a == NULL) { \
						ll_delete_list(ret, free); \
						return NULL; \
					} \
					memcpy(a, al, sizeof(Anchor)); \
					node = ll_new_node((void *)a); \
					if (node == NULL) { \
						free(a); \
						ll_delete_list(ret, free); \
						return NULL; \
					} \
					ll_push_node(ret, node); \
			  } while (0)
		
	beg = alist;
	while (beg->name != NULL && beg->symbol != NULL) {
		add(beg);
		++beg;
	}

#undef add

	return ret;
}
