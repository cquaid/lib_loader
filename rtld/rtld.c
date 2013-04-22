#include <sys/stat.h>
#include <sys/mman.h>

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "rtld.h"
#include "anchor.h"
#include "../debug.h"
#include "../list/list.h"

#define TF_GNU_HASH (1 << 1)
#define TF_HASH     (1 << 2)
#define TF_PLTGOT   (1 << 3)

#define PAGE_LEN (4096)
#define PAGE_MASK (PAGE_LEN - 1)
#define PROT_ALL (PROT_READ | PROT_WRITE | PROT_EXEC)
#define TRUNC_PAGE(vaddr) ((vaddr) & ~PAGE_MASK)
#define ROUND_PAGE(vaddr) (((vaddr) + PAGE_MASK) & ~PAGE_MASK)

typedef void(*init_function)(void);
typedef void(*fini_function)(void);

static Elf_Sym sym_zero;
static Elf_Sym sym_temp;
static elf_object obj_main_0;

static List *fixup_list = NULL;
static List *object_list = NULL;

static void fixup_node_free(void *data);
static void fixup_list_free(void *data);
static void add_object_list(elf_object *obj);
static void free_object_list(void *data);
static void cleanup_object_list(void);
static void fixup_init(void);
static void _rtld_fixup_start(void);
static void* fixup_lookup(char *name);
static Elf_Sym *find_symdef(unsigned long symnum, elf_object *ref_obj,
							elf_object **out, bool in_plt, void *cache);
static int convert_prot(int flags);
#if 0
static Elf_Addr _rtld_fixup(elf_object *obj, Elf_Off reloff);
#endif
static void *dlopen_wrap(char *name, int mode);
static int digest_dynamic(elf_object *obj);
static int _elf_dlreloc(elf_object *obj);
static int _elf_dlmmap(elf_object *obj, int fd, Elf_Ehdr *hdr);
static uint32_t _elf_hash(char *name);
static uint32_t _gnu_hash(char *name);
static void* _gnu_dlsym(elf_object *obj, char *name);
static void* _elf_dlsym(elf_object *obj, char *name);


void
add_fixup_list(List *list)
{
	ListNode *tmp;

	if (list == NULL)
		return;
	
	if (fixup_list == NULL) {
		fixup_list = ll_new_list();
		if (fixup_list == NULL) {
			debugln("couldn't mallocate");
			return;
		}
	}

	tmp = ll_new_node((void *)list);
	if (tmp == NULL) {
		debugln("couldn't mallocate node");
		return;
	}

	ll_push_node(fixup_list, tmp);
}

void
cleanup_fixup_list(void)
{
	ll_delete_list(fixup_list, fixup_list_free);
	fixup_list = NULL;
}

int
elf_dlclose(elf_object *obj)
{
	unsigned long idx;
	fini_function fini;

	if (obj == NULL) {
		debugln("null object");
		return -EINVAL;
	}

	if (obj->fini) {
		fini = (fini_function)obj->fini;
		fini();
	}

	if (obj->dl_count > 0 && obj->dl_handles != NULL) {
		for (idx = obj->dl_count; idx > 0; --idx) {
			if (obj->dl_handles[idx - 1] != NULL)
				elf_dlclose((elf_object* )obj->dl_handles[idx - 1]);
			free(obj->dl_handles);
		}
	}

	munmap(obj->relocbase, obj->relocsize);
	free(obj);

	cleanup_object_list(); /* XXX: this might come back to haunt us */

	return 0;
}

void*
elf_dlsym(elf_object *obj, char *name)
{
	if ((obj->flags & TF_GNU_HASH))
		return _gnu_dlsym(obj, name);
	
	return _elf_dlsym(obj, name);
}

static void*
_gnu_dlsym(elf_object *obj, char *name)
{
	Elf_Hashelt bucket;
	Elf_Addr bloom_word;
	unsigned long h1, h2;
	const int c = BLOOM_SIZE_CONST * 32;
	unsigned long symnum;
	unsigned long hash;
	Elf_Hashelt *hashval;
	char *strp;
	Elf_Sym *symp;

	hash = (unsigned long)(_gnu_hash(name) & 0xffffffff);
	bloom_word = obj->bloom_gnu[(hash / c) & obj->maskwords_bm_gnu];

	/* calculate mod 32/64 of gnu hash and derivative */
	h1 = hash & (c - 1);
	h2 = ((hash >> obj->shift2_gnu) & (c - 1));

	/* filter out things that arn't in the set */
	if (((bloom_word >> h1) & (bloom_word >> h2) & 1) == 0) {
		debug("%s: `%s' is not in the list\n", __func__, name);
		return NULL;
	}

	if (!obj->nbuckets_gnu) {
		debugln("nbuckets_gnu == 0");
		return NULL;
	}
	
	/* locate hash chain */
	bucket = obj->buckets_gnu[hash % obj->nbuckets_gnu];
	if (!bucket) {
		debugln("couldn't find bucket");
		return NULL;
	}

	hashval = &obj->chain_zero_gnu[bucket];
	do {
		if (((*hashval ^ hash) >> 1) == 0) {
			symnum = hashval - obj->chain_zero_gnu;
			symp = obj->symtab + symnum;
			strp = obj->strtab + symp->st_name;

			switch(ELF_ST_TYPE(symp->st_info)) {
			case STT_FUNC:
			case STT_NOTYPE:
			case STT_OBJECT:
			case STT_COMMON:
			case STT_GNU_IFUNC:
				if (!symp->st_value)
					continue;
				/* fall through */
			case STT_TLS:
				if (symp->st_shndx != SHN_UNDEF)
					break;
				/* because of fall through */
				if (ELF_ST_TYPE(symp->st_info) == STT_FUNC)
					break;
				/* fall through */
			default:
				continue;
			}

			if (*name != *strp || strcmp(name, strp))
				continue;

			return (obj->relocbase + symp->st_value);
		}
	} while ((*hashval++ & 1) == 0);

	return NULL;
}

static void*
_elf_dlsym(elf_object *obj, char *name)
{
	unsigned long symnum;
	unsigned long hash;
	char *strp;
	Elf_Sym *symp;

	if (obj == NULL || name == NULL) {
		debugln("null obj or name");
		return NULL;
	}

	if (!(obj->flags & TF_HASH)) {
		debugln("DT_HASH never found");
		return NULL;
	}

	if (!obj->nbuckets) {
		debugln("nbuckets == 0");
		return NULL;
	}

	hash = (unsigned long)(_elf_hash(name) & 0xffffffff);
	symnum = (unsigned long)obj->buckets[hash % obj->nbuckets];

	for (; symnum != SHN_UNDEF; symnum = obj->chains[symnum]) {
		if (symnum >= obj->nchains) {
			debug("%s: symnum (%lu) too large\n", __func__, symnum);
			return NULL;
		}

		symp = obj->symtab + symnum;
		strp = obj->strtab + symp->st_name;

		switch (ELF_ST_TYPE(symp->st_info)) {
		case STT_FUNC:
		case STT_NOTYPE:
		case STT_OBJECT:
		case STT_COMMON:
		case STT_GNU_IFUNC:
			if (!symp->st_value)
				continue;
			/* fall through */
		case STT_TLS:
			if (symp->st_shndx != SHN_UNDEF)
				break;
			/* because of fall through */
			if(ELF_ST_TYPE(symp->st_info) == STT_FUNC)
				break;
			/* fall through */
		default:
			continue;
		}

		if (*name != *strp || strcmp(name, strp))
			continue;
		
		return (obj->relocbase + symp->st_value);
	}

	return NULL;
}

elf_object*
elf_dlopen(char *path)
{
	int fd;
	struct stat st;
	ssize_t nbytes;
	elf_object *ret;
	init_function init;
	
	union {
		Elf_Ehdr hdr;
		char buf[PAGE_LEN];
	} u;

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		debug("%s: open(): %s\n", __func__, strerror(errno));
		return NULL;
	}

	if (fstat(fd, &st) == -1) {
		close(fd);
		debugln("fstat() failed");
		return NULL;
	}

	nbytes = pread(fd, u.buf, PAGE_LEN, 0);
	if (nbytes == -1) {
		close(fd);
		debug("%s: pread(): %s\n", __func__, strerror(errno));
		return NULL;
	}

	if (nbytes < (ssize_t)sizeof(Elf_Ehdr)) {
		close(fd);
		debugln("file too small");
		return NULL;
	}
	
	if (u.hdr.e_phentsize != sizeof(Elf_Phdr)) {
		close(fd);
		debugln("e_phentsize is not sizeof(Elf_Phdr)");
		return NULL;
	}

	if (u.hdr.e_phoff + u.hdr.e_phnum * sizeof(Elf_Phdr) > (size_t)nbytes) {
		close(fd);
		debugln("something too large");
		return NULL;
	}

	ret = (elf_object *)malloc(sizeof(elf_object));
	if (ret == NULL) {
		close(fd);
		debug("%s: malloc(): %s\n", __func__, strerror(errno));
		return NULL;
	}

	memset(ret, 0, sizeof(elf_object));

	if (_elf_dlmmap(ret, fd, &u.hdr)) {
		close(fd);
		free(ret);
		debugln("_elf_dlmmap()");
		return NULL;
	}

	close(fd);
	fixup_init();

	if (digest_dynamic(ret)) {
		debugln("digest_dynamic()");
		munmap(ret->relocbase, ret->relocsize);
		free(ret);
		return NULL;
	}

	if (_elf_dlreloc(ret)) {
		debugln("_elf_dlreloc()");
		munmap(ret->relocbase, ret->relocsize);
		free(ret);
		return NULL;
	}

	if (ret->init) {
		init = (init_function)ret->init;
		init();
	}

	add_object_list(ret);

	return ret;
}

static int
convert_prot(int flags)
{
	int prot = 0;

	prot |= PROT_READ  * !!(flags & PF_R);
	prot |= PROT_WRITE * !!(flags & PF_W);
	prot |= PROT_EXEC  * !!(flags & PF_X);

	return prot;
}

static uint32_t
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

static uint32_t
_gnu_hash(char *name)
{
	unsigned char *p;
	uint_fast32_t h;
	
	p = (unsigned char *)name;
	for (h = 5381; *p != '\0'; ++p)
		h = ((h << 5) + h) + *p;
	
	return (uint32_t)(h & 0xffffffff);
}

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
		debug("%s: obj is null\n", __func__);
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
		debug("%s: symbol missing: %s\n", __func__, obj->strtab + def->st_name);
		return 0; /* XXX: should fail/exit */
	}

	symp = obj->symtab + ELF_R_SYM(rel->r_info);
	name = obj->strtab + symp->st_name;

	*where = (Elf_Addr)(def_obj->relocbase + def->st_value);

	return *where;
}
#endif

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
	
	debug("%s: dlopen_fixup: %s\n", __func__, buf);
	return (void *)elf_dlopen(buf);
}

static Elf_Sym*
find_symdef(unsigned long symnum, elf_object *ref_obj,
		    elf_object **out, bool in_plt, void *cache)
{
	void *symval;
	char *name;
	Elf_Sym *ref;
	Elf_Sym *def;
	elf_object *def_obj;
	
	if (symnum >= ref_obj->dynsymcount) { /* ->nchains */
		debugln("chain too small");
		return NULL;	
	}

	ref = ref_obj->symtab + symnum;
	name = ref_obj->strtab + ref->st_name;
	def_obj = NULL;

	if (ELF_ST_TYPE(ref->st_info) == STT_SECTION) {
		debugln("STT_SECTION");
		return NULL; /* XXX: should fail/exit */
	}

	if (ELF_ST_BIND(ref->st_info) == STB_LOCAL) {
		def = ref;
		def_obj = ref_obj;
		if (!ref->st_value) {
			debugln("ref->st_value == 0");
			return NULL; /* XXX: should fail/exit */
		}
	}
	else if ((symval = fixup_lookup(name)) != NULL) {
		def = &sym_temp;
		def_obj = &obj_main_0;
		sym_temp.st_value = (Elf_Addr)symval;
	}
#if 0
	else if((symval = elf_dlsym(ref_obj, name)) != NULL) {
		def = &sym_temp;
		def_obj = &obj_main_0;
		sym_temp.st_value = (Elf_Addr)symval;
	}
#endif
	else
		def = NULL;

#if 0
	if (def == NULL) {
		symval = dlfunc(RTLD_DEFAULT, name);
		if (symval == NULL)
			break;
		def = &sym_temp;
		def_obj = &obj_main_0;
		sym_temp.st_value = (Elf_Addr)symval;
		break;
	}
#endif

	if (def == NULL && ELF_ST_BIND(ref->st_info) == STB_WEAK) {
		debug("%s: unreferenced weak object: %s\n", __func__, name);
		def = &sym_zero;
		def_obj = &obj_main_0;
	}

	if (def != NULL)
		*out = def_obj;
	else if (!in_plt){
		debug("%s: roc_slot missing: %d %d %s\n", __func__,
			  ELF_ST_BIND(ref->st_info), ELF_ST_TYPE(ref->st_info),
			  name);
	}

	if (def == NULL && !in_plt) {
		debug("%s: symbol: %s\n", __func__, name);
		return NULL; /* XXX: should fail/exit */
	}

	return def;
}

static int
digest_dynamic(elf_object *obj)
{
	int plttype;
	Elf_Dyn *dynp;
	unsigned long dl_count;
	Elf_Hashelt *hashtab;

	if (obj == NULL) {
		debugln("null object");
		return -EINVAL; /* XXX: should fail/exit */
	}

	plttype = 0;
	dl_count = 0;
	for (dynp = obj->dynamic; dynp->d_tag != DT_NULL; ++dynp) {
		switch (dynp->d_tag) {
		case DT_NEEDED:
			++dl_count;
			break;

		case DT_SONAME:
			/* ignoring */
			break;

		case DT_PLTRELSZ:
			obj->pltrelsize = dynp->d_un.d_val;
			break;

		case DT_PLTGOT:
			obj->pltgot = (Elf_Addr *)(obj->relocbase + dynp->d_un.d_ptr);
			obj->flags |= TF_PLTGOT;
			break;

		case DT_GNU_HASH:
			{
				Elf32_Word nmaskword;
				int bloom_size32;

				obj->flags |= TF_GNU_HASH;
				hashtab = (Elf_Hashelt *)(obj->relocbase + dynp->d_un.d_ptr);
				obj->nbuckets_gnu = hashtab[0];
				obj->symndx_gnu = hashtab[1];
				nmaskword = hashtab[2];
				bloom_size32 = (BLOOM_SIZE_CONST) * nmaskword;
				obj->maskwords_bm_gnu = nmaskword - 1;
				obj->shift2_gnu = hashtab[3];
				obj->bloom_gnu = (Elf_Addr *)(hashtab + 4);
				obj->buckets_gnu = hashtab + 4 + bloom_size32;
				obj->chain_zero_gnu = obj->buckets_gnu + obj->nbuckets_gnu
									- obj->symndx_gnu;
			}
			break;

		case DT_HASH:
			hashtab = (Elf_Hashelt *)(obj->relocbase + dynp->d_un.d_ptr);
			obj->nbuckets = hashtab[0];
			obj->nchains = hashtab[1];
			obj->buckets = &hashtab[2];
			obj->chains = &obj->buckets[obj->nbuckets];
			obj->flags |= TF_HASH;
			break;

		case DT_STRTAB:
			obj->strtab = (char *)(obj->relocbase + dynp->d_un.d_ptr);
			break;

		case DT_SYMTAB:
			obj->symtab = (Elf_Sym *)(obj->relocbase + dynp->d_un.d_ptr);
			break;

		case DT_STRSZ:
			obj->strsize = dynp->d_un.d_val;
			break;
		
		case DT_SYMENT:
			/* ignoring */
			break;
		
		case DT_INIT_ARRAY:
		case DT_FINI_ARRAY:
		case DT_INIT_ARRAYSZ:
		case DT_FINI_ARRAYSZ:
			/* ignoring? */
			break;

		case DT_DEBUG:
			/* we don't need no strinkin' debug syms */
			break;

		case DT_VERSYM:
		case DT_VERDEF:
		case DT_VERDEFNUM:
		case DT_VERNEED:
		case DT_VERNEEDNUM:
		case DT_RELCOUNT:
		case DT_RELACOUNT:
			/* ignoring */
			break;
		
		case DT_INIT:
			obj->init = (Elf_Addr)(obj->relocbase + dynp->d_un.d_ptr);
			break;
		
		case DT_FINI:
			obj->fini = (Elf_Addr)(obj->relocbase + dynp->d_un.d_ptr);
			break;
		
		case DT_REL:
			obj->rel = (Elf_Rel *)(obj->relocbase + dynp->d_un.d_ptr);
			break;
		
		case DT_RELSZ:
			obj->relsize = (unsigned long)dynp->d_un.d_val;
			break;
		
		case DT_RELA:
			obj->rela = (Elf_Rela *)(obj->relocbase + dynp->d_un.d_ptr);
			break;

		case DT_RELASZ:
			obj->relasize = (unsigned long)dynp->d_un.d_val;
			break;
		
		case DT_RELAENT:
			/* ignoring */
			break;

		case DT_PLTREL:
			plttype = dynp->d_un.d_val;
			break;
		
		case DT_JMPREL:
			obj->pltrel = (Elf_Rel *)(obj->relocbase + dynp->d_un.d_val);
			break;
		
		default:
			debug("%s: unknown dt_type %lx\n", __func__, (unsigned long)dynp->d_tag);
			break;	
		}
	}

	if (dl_count > 0) {
		obj->dl_count = dl_count;
		obj->dl_handles = (void **)malloc(dl_count * sizeof(void *));
		memset(obj->dl_handles, 0, dl_count * sizeof(void *));
		dl_count = 0;
	}

	for (dynp = obj->dynamic; dynp->d_tag != DT_NULL; ++dynp) {
		char *strp;
		switch (dynp->d_tag) {
		case DT_NEEDED:
			strp = (char *)(obj->strtab + dynp->d_un.d_ptr);
			obj->dl_handles[dl_count] = dlopen_wrap(strp, 0);
			++dl_count;
			debug("%s: needed: %s\n", __func__, strp);
			break;
		
		case DT_SONAME:
			debug("%s: soname: %s\n", __func__, obj->strtab +dynp->d_un.d_ptr);
			break;

		default:
			break;
		}
	}

	if (plttype == DT_RELA) {
		obj->pltrela = (Elf_Rela *)obj->pltrel;
		obj->pltrelasize = obj->pltrelsize;
		obj->pltrel = 0;
		obj->pltrelsize = 0;
	}

#if 1
	if (obj->flags & TF_HASH)
		obj->dynsymcount = obj->nchains;
	else if (obj->flags & TF_GNU_HASH) {
		Elf_Hashelt *hashval;
		unsigned long i;
		obj->dynsymcount = 0;
		for (i = 0; i < obj->nbuckets_gnu; ++i) {
			if (!obj->buckets_gnu[i])
				continue;
			hashval =  &obj->chain_zero_gnu[obj->buckets_gnu[i]];
			do {
				obj->dynsymcount++;
			} while ((*hashval++ & 0x1) == 0);
		}
		obj->dynsymcount += obj->symndx_gnu;
	}
#endif
	return 0;
}

static int
_elf_dlreloc(elf_object *obj)
{
	Elf_Addr *where;
	Elf_Sym *def;
	Elf_Rela *rela;
	Elf_Rela *relalim;
	Elf_Rela *pltrela;
	Elf_Rela *pltrelalim;
	Elf_Rel *rel;
	Elf_Rel *rellim;
	Elf_Rel *pltrel;
	Elf_Rel *pltrellim;
	elf_object *def_obj;

	relalim = obj->rela + (obj->relasize / sizeof(Elf_Rela));
	for (rela = obj->rela; rela < relalim; ++rela) {
		where = (Elf_Addr *)(obj->relocbase + rela->r_offset);

		switch (ELF_R_TYPE(rela->r_info)) {
		case R_X86_64_64:
			def = find_symdef(ELF_R_SYM(rela->r_info), obj, &def_obj,
							  false, NULL);
			if (def == NULL) {
				debugln("R_X86_64_64: def == NULL");
				return -1; /* XXX: should fail/exit */
			}
			*where = (Elf_Addr)(def_obj->relocbase + def->st_value + rela->r_addend);
			break; /* XXX: should this fall through? */
		
		case R_X86_64_GLOB_DAT:
			def = find_symdef(ELF_R_SYM(rela->r_info), obj, &def_obj,
							  false, NULL);
			if (def == NULL) {
				debugln("R_X86_64_GLOB_DAT: def == NULL");
				return -1; /* XXX: should fail/exit */
			}
			*where = (Elf_Addr)(def_obj->relocbase + def->st_value);
			break;

		case R_X86_64_RELATIVE:
			*where = (Elf_Addr)(obj->relocbase + rela->r_addend);
			break;

		default:
			debug("%s: rel slot drop: type %lx, bind %lx\n", __func__,
				  (unsigned long)ELF_R_TYPE(rela->r_info),
				  (unsigned long)ELF_ST_BIND(rela->r_info));
			break;
		}
	}

	pltrelalim = obj->pltrela + (obj->pltrelasize / sizeof(Elf_Rela));
	for (pltrela = obj->pltrela; pltrela < pltrelalim; ++pltrela) {
		where = (Elf_Addr *)(obj->relocbase + pltrela->r_offset);
		*where += (Elf_Addr)(obj->relocbase);
	}

#if 1
	for (pltrela = obj->pltrela; pltrela < pltrelalim; ++pltrela) {
		where = (Elf_Addr *)(obj->relocbase + pltrela->r_offset);
		def = find_symdef(ELF_R_SYM(pltrela->r_info), obj, &def_obj,
						  true, NULL);
		if (def != NULL)
			*where = (Elf_Addr)(def_obj->relocbase + def->st_value + pltrela->r_addend);
	}
#endif

	rellim = obj->rel + (obj->relsize / sizeof(Elf_Rel));
	for (rel = obj->rel; rel < rellim; ++rel) {
		where = (Elf_Addr *)(obj->relocbase + rel->r_offset);

		switch (ELF_R_TYPE(rel->r_info)) {
		case R_386_NONE:
			/* ignore */
			break;

		case R_386_GLOB_DAT:
			def = find_symdef(ELF_R_SYM(rel->r_info), obj, &def_obj,
							  false, NULL);
			if (def == NULL) {
				debugln("R_386_GLOB_DAT: def == null");
				return -1; /* XXX: should fail/exit */
			}
			*where = (Elf_Addr)(def_obj->relocbase + def->st_value);
			break;

		case R_386_RELATIVE:
			*where += (Elf_Addr)obj->relocbase;
			break;

		case R_386_32:
			def = find_symdef(ELF_R_SYM(rel->r_info), obj, &def_obj,
							  false, NULL);
			if (def == NULL) {
				debugln("R_386_32: def == null");
				return -1; /* XXX: should fail/exit */
			}
			*where = (Elf_Addr)(def_obj->relocbase + def->st_value);
			break;
		
		case R_386_TLS_DTPMOD32:
			/* ignoring */
			break;

		default:
			debug("%s: unknown type: %lu\n", __func__,
				  (unsigned long)ELF_R_TYPE(rel->r_info));
			return -1; /* XXX: should fail/exit */
			break;
		}
	}

	pltrellim = obj->pltrel + (obj->pltrelsize / sizeof(Elf_Rel));
	for (pltrel = obj->pltrel; pltrel < pltrellim; ++pltrel) {
		where = (Elf_Addr *)(obj->relocbase + pltrel->r_offset);
		*where += (Elf_Addr)(obj->relocbase);
	}

	for (pltrel = obj->pltrel; pltrel < pltrellim; ++pltrel) {
		where = (Elf_Addr *)(obj->relocbase + pltrel->r_offset);
		def = find_symdef(ELF_R_SYM(pltrel->r_info), obj, &def_obj,
						  true, NULL);
		if (def != NULL)
			*where = (Elf_Addr)(def_obj->relocbase + def->st_value);
	}

	if (obj->flags & TF_PLTGOT) {
		obj->pltgot[2] = (Elf_Addr)_rtld_fixup_start;
		obj->pltgot[1] = (Elf_Addr)obj;
	}

	return 0;
}

static int
_elf_dlmmap(elf_object *obj, int fd, Elf_Ehdr *hdr)
{
	int pltloadfirst;
	Elf_Phdr *phdr;
	Elf_Phdr *phdyn;
	Elf_Phdr *phdrinit;
	Elf_Phdr *phdrfini;
	Elf_Addr base_vaddr;
	Elf_Addr base_vlimit;
	Elf_Addr mapbase;
	size_t mapsize;

	phdrinit = (Elf_Phdr *)((char *)hdr + hdr->e_phoff);
	phdrfini = (Elf_Phdr *)(phdrinit + hdr->e_phnum);

	for (phdr = phdrinit; phdr < phdrfini; ++phdr) {
		switch (phdr->p_type) {
		case PT_INTERP:
			/* ignore */
			break;

		case PT_LOAD:
			if (!pltloadfirst)
				base_vaddr = TRUNC_PAGE(phdr->p_vaddr);
			base_vlimit = ROUND_PAGE(phdr->p_vaddr + phdr->p_memsz);
			++pltloadfirst;
			break;

		case PT_PHDR:
			/* ignore */
			break;

		case PT_DYNAMIC:
			phdyn = phdr;
			break;

		case PT_TLS:
			/* ignore */
			break;

		default:
			/* ignore */
			break;
		}
	}

	mapsize = (size_t)(base_vlimit - base_vaddr);
	mapbase = (Elf_Addr)mmap(NULL, mapsize, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mapbase == (Elf_Addr)MAP_FAILED) {
		debug("%s: mmap(): %s\n", __func__, strerror(errno));
		return -1;
	}

	for (phdr = phdrinit; phdr < phdrfini; ++phdr) {
		Elf_Off data_off;
		Elf_Addr data_addr;
		Elf_Addr data_vaddr;
		Elf_Addr data_vlimit;
		int data_prot;
		int data_flags;

		if (phdr->p_type != PT_LOAD)
			continue;

		data_off = TRUNC_PAGE(phdr->p_offset);
		data_vaddr = TRUNC_PAGE(phdr->p_vaddr);
		data_vlimit = ROUND_PAGE(phdr->p_vaddr + phdr->p_filesz);

		data_addr = mapbase + (data_vaddr - base_vaddr);
		data_prot = convert_prot(phdr->p_flags);
		data_flags = MAP_FIXED | MAP_PRIVATE;

		if (mmap((void *)data_addr, data_vlimit - data_vaddr,
				 data_prot | PROT_WRITE, data_flags, fd,
				 data_off)== MAP_FAILED) {
			debug("%s: mmap(data_addr): %s\n", __func__, strerror(errno));
			return -1;
		}

		if (phdr->p_filesz != phdr->p_memsz) {
			ssize_t nclear;
			void *clear_page;
			Elf_Addr clear_vaddr;
			Elf_Addr clear_addr;

			clear_vaddr = phdr->p_vaddr + phdr->p_filesz;
			clear_addr = mapbase + (clear_vaddr - base_vaddr);
			clear_page = (void *)(mapbase + (TRUNC_PAGE(clear_vaddr) - base_vaddr));

			if ((nclear = data_vlimit - clear_vaddr) > 0) {
				Elf_Addr bss_vaddr;
				Elf_Addr bss_addr;
				Elf_Addr bss_vlimit;

				if (!(data_prot & PROT_WRITE))
					mprotect(clear_page, PAGE_LEN, data_prot | PROT_WRITE);
				
				memset((void *)clear_addr, 0, nclear);

				if (!(data_prot & PROT_WRITE))
					mprotect(clear_page, PAGE_LEN, data_prot);

				bss_vaddr = data_vlimit;
				bss_vlimit = ROUND_PAGE(phdr->p_vaddr + phdr->p_memsz);
				bss_addr = mapbase + (bss_vaddr - base_vaddr);

				if (mprotect((void *)bss_addr, bss_vlimit - bss_vaddr, data_prot) == -1) {
					debug("%s: mprotect(): %s\b", __func__, strerror(errno));
					return -1;
				}
			}
		}
	}

	obj->flags = 0;
	obj->relocsize = mapsize;
	obj->relocbase = (unsigned char *)mapbase;
	obj->dl_count = 0;
	obj->dl_handles = NULL;
	obj->dynamic = (Elf_Dyn *)(phdyn->p_vaddr + obj->relocbase);

	return 0;
}

static void free_object_list(void *data){}
static void fixup_init(void){}
static void _rtld_fixup_start(void){}

static void*
fixup_lookup(char *name)
{
	Anchor *a;
	ListNode *c;
	
	if (fixup_list == NULL && object_list == NULL) {
		debug("%s: WARN: `%s' not found\n", __func__, name);
		return NULL;
	}
	
	if (fixup_list == NULL)
		goto object_search;

	c = fixup_list->head;
	for (; c != NULL; c = c->next) {
		List *l = c->data;
		ListNode *n;
		
		if (l == NULL)
			continue;
		
		n = l->head;
		for (; n != NULL; n = n->next) {
			a = n->data;
			if (a == NULL)
				continue;

			if (!strcmp(a->name, name)) {
				debug("%s: INFO: `%s' found in fixup_list\n", __func__, name);
				return a->symbol;
			}
		}
	}

object_search:
	if (object_list == NULL)
		goto out;

	c = object_list->head;
	for (; c != NULL; c = c->next) {
		void *d = elf_dlsym((elf_object *)(c->data), name);
		if (d != NULL) {
			debug("%s: INFO: `%s' found in object_list\n", __func__, name);
			return d;
		}
	}

out:
	debug("%s: WARN: `%s' not found\n", __func__, name);
	return NULL;
}

static void
fixup_node_free(void *data)
{
	free(data);
}

static void
fixup_list_free(void *data)
{
	ll_delete_list((List *)data, fixup_node_free);
}

static void
add_object_list(elf_object *obj)
{
	ListNode *tmp;

	if (obj == NULL)
		return;

	if (object_list == NULL) {
		object_list = ll_new_list();
		if (object_list == NULL) {
			debugln("coulsn't mallocate");
			return;
		}
	}

	tmp = ll_new_node((void*)obj);
	if (tmp == NULL) {
		debugln("couldn't mallocate node");
		return;
	}
	
	ll_push_node(object_list, tmp);
}

static void
cleanup_object_list(void)
{
	ll_delete_list(object_list, free_object_list);
	object_list = NULL;
}
