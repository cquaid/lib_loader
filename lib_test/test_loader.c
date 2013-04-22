#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../rtld/anchor.h"
#include "../rtld/rtld.h"

Anchor a = { "malloc", malloc };

typedef int(*getRandomNumber)(void);
typedef char*(*getString)(void);

int
main(int argc, char *argv[])
{
	elf_object *elf;
	getRandomNumber n;
	getString s;
	char *t;

	add_fixup_anchor(&a);

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

	return 0;
}
