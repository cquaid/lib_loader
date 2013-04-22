#ifndef DEBUG
#define debugln(s) do{}while(0)
#define debug(...) do{}while(0)
#else
#include <stdio.h>
#define debugln(s) do{fprintf(stderr,"%s: %s\n",__func__,s);}while(0)
#define debug(...) do{fprintf(stderr,__VA_ARGS__);}while(0)
#endif
