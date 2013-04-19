#ifndef DEBUG
//#define debug(s) do{}while(0)
#define debug(...) do{}while(0)
#else
#include <stdio.h>
//#define debug(s) do{fprintf(stderr,"%s",s);}while(0)
#define debug(...) do{fprintf(stderr,__VA_ARGS__);}while(0)
#endif
