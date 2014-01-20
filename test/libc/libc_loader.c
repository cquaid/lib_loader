#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "librtld.h"

#ifndef LIBC_PATH
# error "LIBC_PATH unset!"
#endif

#ifndef LD_PATH
# error "LD_PATH unset!"
#endif

#ifdef DEBUG
# define debug(...) do{ printf(__VA_ARGS__); }while(0)
#else
# define debug(...) do{}while(0)
#endif

static char *ldso = LD_PATH;
static char *libc = LIBC_PATH;

static elf_object *ld_elf = NULL;
static elf_object *libc_elf = NULL;

static void die(const char *fmt, ...);
static elf_object* xelf_dlopen(char *path);
static void* xelf_dlsym(elf_object *elf, char *sym);

typedef int(*printf_fn)(const char*, ...);

int
main(int argc, char *argv[])
{
	printf_fn pf;

	ld_elf = xelf_dlopen(ldso);
	libc_elf = xelf_dlopen(libc);

	pf = (printf_fn)xelf_dlsym(libc_elf, "printf");
	pf("Hello World\n");

	elf_dlclose(libc_elf);
	elf_dlclose(ld_elf);

	return 0;
}

static void
die(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	if (libc_elf)
		elf_dlclose(libc_elf);

	if (ld_elf)
		elf_dlclose(ld_elf);

	exit(EXIT_FAILURE);
}

static elf_object*
xelf_dlopen(char *path)
{
	elf_object *ret;

	ret = elf_dlopen(path);
	if (ret == NULL)
		die("elf_dlopen(%s) failed\n", path);
	debug("Successfully loaded %s\n", path);

	return ret;
}

static void*
xelf_dlsym(elf_object *elf, char *sym)
{
	void *ret;

	ret = elf_dlsym(elf, sym);
	if (ret == NULL)
		die("elf_dlsym(%s) failed\n", sym);
	debug("Successfully found %s\n", sym);

	return ret;
}
