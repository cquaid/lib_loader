#ifndef _H_RTLD
#define _H_RTLD

#include <elf.h>
#include "anchor.h"

#ifdef __i386__
# define ELF_D(x) typedef Elf32_##x Elf_##x
# define ELF_ST_TYPE(x) ELF32_ST_TYPE(x)
# define ELF_ST_BIND(x) ELF32_ST_BIND(x)
# define ELF_R_TYPE(x)  ELF32_R_TYPE(x)
# define ELF_R_SYM(x)   ELF32_R_SYM(x)
# define BLOOM_SIZE_CONST (1)
typedef Elf32_Word Elf_Xword;
#else
# define ELF_D(x) typedef Elf64_##x Elf_##x
# define ELF_ST_TYPE(x) ELF64_ST_TYPE(x)
# define ELF_ST_BIND(x) ELF64_ST_BIND(x)
# define ELF_R_TYPE(x)  ELF64_R_TYPE(x)
# define ELF_R_SYM(x)   ELF64_R_SYM(x)
# define BLOOM_SIZE_CONST (2)
typedef Elf64_Xword Elf_Xword;
#endif

ELF_D(Word);
ELF_D(Ehdr);
ELF_D(Shdr);
ELF_D(Sym);
ELF_D(Half);
ELF_D(Addr);
ELF_D(Phdr);
ELF_D(Rel);
ELF_D(Rela);
ELF_D(Dyn);
ELF_D(Off);
ELF_D(Section);
ELF_D(Versym);
ELF_D(Sword);
#undef ELF_D


typedef Elf32_Word Elf_Hashelt;

struct _elf_object {
	unsigned long flags;
	Elf_Addr *pltgot;

	Elf_Rel *rel;
	unsigned long relsize;

	Elf_Rela *rela;
	unsigned long relasize;

	Elf_Rel *pltrel;
	unsigned long pltrelsize;

	Elf_Rela *pltrela;
	unsigned long pltrelasize;

	Elf_Sym *symtab;

	char *strtab;
	unsigned long strsize;

	Elf_Hashelt *buckets;
	unsigned long nbuckets;

	Elf_Hashelt *chains;
	unsigned long nchains;

	Elf_Addr init;
	Elf_Addr fini;

	Elf_Addr preinit_array;
	unsigned long preinit_array_len;

	Elf_Addr init_array;
	unsigned long init_array_len;

	Elf_Addr fini_array;
	unsigned long fini_array_len;

	Elf_Dyn *dynamic;
	unsigned long dynsymcount;

	unsigned char *relocbase;
	unsigned long relocsize;

	unsigned long dl_count;
	void **dl_handles;

	/* Everything required for GNU Hash */
	unsigned long nbuckets_gnu;
	unsigned long symndx_gnu;
	unsigned long maskwords_bm_gnu;
	unsigned long shift2_gnu;
	Elf_Addr *bloom_gnu;
	Elf_Hashelt *buckets_gnu;
	Elf_Hashelt *chain_zero_gnu;
};
typedef struct _elf_object elf_object;

extern void add_fixup_anchor(Anchor *anchor);
extern void add_fixup_anchor_list(Anchor *anchor_list);
extern int elf_dlclose(elf_object *obj);
extern void* elf_dlsym(elf_object *obj, char *name);
extern elf_object* elf_dlopen(char *path);

#endif /* _H_RTLD */
