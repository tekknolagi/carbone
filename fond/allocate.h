
#include "configuration.h"
#include <string.h>

void alloc_init(void);
void *allocate(int x);


#if defined(NO_GARBAGE_COLLECTOR)
#include <malloc.h>
#include <stdio.h>

#ifdef DEBOMB
#define mem_env 8
void register_malloc(void *p, int len, char *from, int line);
void check_old_mallocs(void);
#endif

# define realloc(x,y) realloc(x,y)

#else      // !NO_GARBAGE_COLLECTOR

# include <gc/gc.h>
# define realloc(x,y) GC_realloc(x,y)

#endif


#define alloc(type) ((type*)allocate(sizeof(type)))
#define allocn(num, type) ((type*)allocate(sizeof(type)*num))

#define string_dup(str) strcpy(allocate(strlen(str)+1),str)


