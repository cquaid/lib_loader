#include <stdio.h>
#include <stdlib.h>

#include "librtld.h"


static void* my_malloc(size_t len);

Anchor a = { "malloc", my_malloc };

typedef char*(*create_string)(size_t);

int
main(int argc, char *argv[])
{
	char *s;
	elf_object *elf;
	create_string cs;

	add_fixup_anchor(&a);

	elf = elf_dlopen("./malloc_test.so");
	if (elf == NULL) {
		fprintf(stderr, "elf_dlopen failed\n");
		return 1;
	}

	cs = (create_string)elf_dlsym(elf, "create_string");
	if (cs == NULL) {
		fprintf(stderr, "elf_dlsym('create_string') failed\n");
		return 1;
	}

	s = cs(52);
	printf("create_string returned: %s\n", s);

	if (s != NULL)
		free(s);

	elf_dlclose(elf);

	return 0;
}

static void*
my_malloc(size_t len)
{
	printf("%s called, requesting %zd bytes\n", __func__, len);

	return malloc(len);
}
