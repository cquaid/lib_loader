#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "librtld.h"

Anchor a = { "malloc", malloc };

typedef int(*getRandomNumber)(void);
typedef char*(*getString)(void);

static char *lib = "./lib.so";

int
main(int argc, char *argv[])
{
	elf_object *elf;
	getRandomNumber n;
	getString s;
	char *t;

	add_fixup_anchor(&a);

	elf = elf_dlopen(lib);
	if (elf == NULL) {
		fprintf(stderr, "elf_dlopen(%s) failed\n", lib);
		return 1;
	}

	n = (getRandomNumber)elf_dlsym(elf, "getRandomNumber");
	if (n == NULL) {
		fprintf(stderr, "elf_dlsym('getRandomNumber') failed\n");
		elf_dlclose(elf);
		return 1;
	}

	printf("getRandomNumber returned: %x\n", n());

	s = (getString)elf_dlsym(elf, "getString");
	if (s == NULL) {
		fprintf(stderr, "elf_dlsym('getString') failed\n");
		elf_dlclose(elf);
		return 1;
	}

	t = s();
	printf("getString returned: %s\n", t);
	if (t != NULL)
		free(t);

	elf_dlclose(elf);

	return 0;
}
