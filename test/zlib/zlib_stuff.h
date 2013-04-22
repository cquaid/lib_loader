#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void*(*alloc_func)(void*,unsigned int, unsigned int);
typedef void(*free_func)(void*,void*);

struct internal_state{int dummy;};
typedef struct z_stream_s {
	unsigned char *next_in;
	unsigned int avail_in;
	unsigned long total_in;
	unsigned char *next_out;
	unsigned int avail_out;
	unsigned long total_out;
	char *msg;
	struct internal_state *state;
	alloc_func zalloc;
	free_func zfree;
	void* opaque;
	int data_type;
	unsigned long adler;
	unsigned long reserved;
} z_stream;

#define DeflateInit(x,l) deflateInit(x,l,"1.2.7",(int)sizeof(z_stream))
#define InflateInit(x) inflateInit(x,"1.2.7",(int)sizeof(z_stream))
typedef int(*deflateInit_fn)(void*,int,char*,int);
typedef int(*inflateInit_fn)(void*,char*,int);
typedef void(*zerr_fn)(int);
typedef int(*deflateEnd_fn)(void*);
typedef int(*deflate_fn)(void*, int);
typedef int(*inflateEnd_fn)(void*);
typedef int(*inflate_fn)(void*, int);

deflateEnd_fn deflateEnd = NULL;
deflateInit_fn deflateInit = NULL;
deflate_fn deflate = NULL;
inflateInit_fn inflateInit = NULL;
inflateEnd_fn inflateEnd = NULL;
inflate_fn inflate = NULL;

static void
zerr(int ret) {
	fprintf(stderr, "zpipe: \n");
	switch(ret){
		case -1: fprintf(stderr, "err reading/writing to stdin/stdout\n"); break;
		case -2: fprintf(stderr,"invalid compression elvel\n");break;
		case -3: fprintf(stderr,"invalid deflate data\n");break;
		case -4: fprintf(stderr,"out of mem\n");break;
		case -6:
			fprintf(stderr, "version mismatch\n");
	}
}
static int
def(FILE *in, FILE *out, int mode)
{
	int ret, flush;
	unsigned int have;
	z_stream strm;
	unsigned char inb[16384];
	unsigned char outb[16384];

	strm.zalloc = NULL;
	strm.zfree = NULL;
	strm.opaque = NULL;
	ret = DeflateInit(&strm, mode);
	if (ret != 0)
		return ret;
	
	do {
		strm.avail_in = fread(inb, 1, 16384, in);
		if (ferror(in)) {
			(void)deflateEnd(&strm);
			return -1;
		}
		flush = feof(in) ? 4 : 0;
		strm.next_in = inb;

		do {
			strm.avail_out = 16384;
			strm.next_out = outb;
			ret = deflate(&strm, flush);
			have = 16384 - strm.avail_out;
			if (fwrite(outb, 1, have, out) != have || ferror(out)) {
				(void)deflateEnd(&strm);
				return -1;
			}

		}while(strm.avail_out == 0);

	} while (flush != 4);

	(void)deflateEnd(&strm);
	return 0;
}

static int
inf(FILE *in, FILE *out)
{
	int ret;
	unsigned int have;
	z_stream strm;
	unsigned char inb[16384];
	unsigned char outb[16384];

	strm.zalloc = NULL;
	strm.zfree = NULL;
	strm.opaque = NULL;
	strm.avail_in = 0;
	strm.next_in = NULL;
	ret = InflateInit(&strm);
	if (ret != 0)
		return ret;
	
	do{ 
		strm.avail_in = fread(inb, 1, 16384, in);
		if (ferror(in)) {
			(void)inflateEnd(&strm);
			return -1;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = inb;

		do {
			strm.avail_out = 16384;
			strm.next_out = outb;
			ret = inflate(&strm, 0);
			switch(ret) {
			case 2:
				ret = -3;
			case -3:
			case -4:
			(void)inflateEnd(&strm);
			return ret;
			}
			have = 16384 - strm.avail_out;
			if (fwrite(outb, 1, have, out) != have || ferror(out)) {
				(void)inflateEnd(&strm);
				return -1;
			}

		} while (strm.avail_out == 0);

	} while(ret != 1);

	(void)inflateEnd(&strm);
	return ret == 1 ? 0 : -3;
}
