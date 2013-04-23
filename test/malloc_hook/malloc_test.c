#include <stddef.h>
#include <stdlib.h>

char*
create_string(size_t len)
{
	char c;
	size_t i;
	char *ret;
	
	ret = (char *)malloc(len + 1);
	if (ret == NULL)
		return NULL;

	c = 'Z' + 1;
	for (i = 0 ; i < len; ++i) {
		if (c == ('z' + 1))
			c = 'A';
		else if (c == ('Z' + 1))
			c = 'a';

		ret[i] = c++;
	}

	ret[len] = '\0';

	return ret;
}
