#include <sys/mman.h>

#include <elf.h>
#include <stdint.h>

#include "rtld.h"

int
convert_prot(int flags)
{
	int prot = 0;

	/* convert the ELF flags to mmap flags */
	prot |= PROT_READ  * !!(flags & PF_R);
	prot |= PROT_WRITE * !!(flags & PF_W);
	prot |= PROT_EXEC  * !!(flags & PF_X);

	return prot;
}

uint32_t
_elf_hash(char *name)
{
	unsigned char *p;
	uint32_t h, g;

	p = (unsigned char *)name;
	for (h = 0; *p != '\0'; ++p) {
		h = (h << 4) + *p;
		if ((g = h & 0xf0000000))
			h ^= g >> 24;
		h &= ~g;
	}

	return h;
}

uint32_t
_gnu_hash(char *name)
{
	unsigned char *p;
	uint_fast32_t h;

	p = (unsigned char *)name;
	for (h = 5381; *p != '\0'; ++p)
		h = ((h << 5) + h) + *p;

	return (uint32_t)(h & 0xffffffff);
}


/**
 * Useless things
 */
#if 0
static Elf_Addr
_rtld_fixup(elf_object *obj, Elf_Off reloff)
{
	char *name;
	Elf_Sym *def;
	Elf_Addr *where;
	Elf_Rel *rel;
	Elf_Sym *symp;
	elf_object *def_obj;

	if (obj == NULL) {
		debug("obj is null\n");
		return 0; /* XXX: should fail/exit */
	}

	if (obj->pltrel != NULL)
		rel = (Elf_Rel *)((char *)obj->pltrel + reloff);
	else
		rel = (Elf_Rel *)((char *)obj->pltrela + reloff);

	where = (Elf_Addr *)(obj->relocbase + rel->r_offset);

	def = find_symdef(ELF_R_SYM(rel->r_info), obj, &def_obj, NULL);
	if (def == NULL) {
		def = obj->symtab + ELF_R_SYM(rel->r_info);
		debug("symbol missing: %s\n", obj->strtab + def->st_name);
		return 0; /* XXX: should fail/exit */
	}

	symp = obj->symtab + ELF_R_SYM(rel->r_info);
	name = obj->strtab + symp->st_name;

	*where = (Elf_Addr)(def_obj->relocbase + def->st_value);

	return *where;
}

/* XXX: currently this function is useless
 * need to get the LD_LIBRARY_PATH env variable
 * for search paths and need to contain all
 * symbols that ld.so exports. */
static void*
dlopen_wrap(char *name, int mode)
{
	void *ret;
	char buf[1024];
	char *p;

	(void)mode;

	ret = (void *)elf_dlopen(name);
	if (ret != NULL)
		return ret;

	strncpy(buf, name, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

	p = strstr(buf, ".so");
	if (p != NULL)
		*(p + 3) = '\0';

	return (void *)elf_dlopen(buf);
}
#endif
