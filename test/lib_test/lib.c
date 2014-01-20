/**
 * --static--
 * gcc -c lib.c
 * ar -cvq lib.a lib.o
 * --dynamic--  # -fPIC is for Position Independent Code
 *			    # -fpic is the same
 * gcc -fPIC -c lib.c
 * gcc -shared -Wl,-soname,lib.so.1 -o lib.so.1.0 lib.o
 */

static int c = 0x30;
#include <stdlib.h>


int getRandomNumber(void)
{
	c += 2;
	return c; /* chosen by fair dice roll. */
			  /* guaranteed to be random.  */
}

char* getString(void)
{
	char* s = (char*)malloc(2);
	s[0] = 'H'; s[1] = '\0';
	return s;
}
