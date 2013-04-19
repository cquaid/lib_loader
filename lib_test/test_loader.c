#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../rtld/rtld.h"
#include "../list/list.h"

Anchor a = { "malloc", malloc };

typedef int(*getRandomNumber)(void);
typedef char*(*getString)(void);

int
main(int argc, char *argv[])
{
	elf_object *elf;
	getRandomNumber n;
	getString s;
	List *l;
	ListNode *no;
	char *t;
	Anchor *b;


	l = ll_new_list();
	if (l == NULL)
		return -1;
	
	b = (Anchor *)malloc(sizeof(Anchor));
	if (b == NULL) {
		free(l);
		return -1;
	}
	memcpy(b, &a, sizeof(Anchor));
	no = ll_new_node((void *)b);
	if (no == NULL) {
		free(b);
		free(l);
		return -1;
	}

	ll_push_node(l, no);

	add_fixup_list(l);

	elf = elf_dlopen(argv[1]);
	n = (getRandomNumber)elf_dlsym(elf, "getRandomNumber");
	printf("%p\n",n);
	if (n != NULL)
		printf("%x %x\n", n(), n());
	s = (getString)elf_dlsym(elf, "getString");
	printf("%p\n", s);
	if (s != NULL) {
		t = s();
		printf("%s\n",t);
		free(t);
	}
	elf_dlclose(elf);

	cleanup_fixup_list();
	return 0;
}
