
#include "fond/allocate.h"


void alloc_init(void) {
  #if !defined(NO_GARBAGE_COLLECTOR) && defined(GC_INCREMENTAL)
    GC_enable_incremental();
  #endif
}



#if !defined(NO_GARBAGE_COLLECTOR)

void *allocate(int x) {
  return GC_malloc(x);
}


#else

#  if !defined(DEBOMB)


void *allocate(int x) {
  return (memset(malloc(x), 0, x));
}


#  else

void *allocate(int x) {
  void *m = memset(malloc(x+mem_env), 0, x+mem_env);
  register_malloc(m, x, "",0);
  check_old_mallocs();
  return(  ((char*)m)+(mem_env/2)  );
}


#include <stdio.h>
#include <assert.h>
#define num_allocs 100000

typedef struct MALL {
  long *adr;
  long len;
  char *from;
  int line;
  int  reported;
} MALL;

MALL all_mallocs[num_allocs];
int curr_malloc = 0;

void register_malloc(void *p, int len, char *from, int line) {
  int j, offs;
  MALL *m = all_mallocs + curr_malloc++;
  char *a = p;
  m->adr = p;
  m->len = len;
  m->from = from;
  m->line = line;
  m->reported = 0;

  assert ( curr_malloc < num_allocs );

  for (j = 0; j < mem_env; j++) {
    if (j < mem_env/2) offs = j; else offs = m->len + j;
    a[offs] = offs%120;
  }
}

void dump_mem(MALL *m);
void check_old_mallocs() {
  int i, j, offs;
  MALL *m;
  char *a;

  for (i = 0; i < curr_malloc; i++) {
    m = all_mallocs + i;
    a = (char*) m->adr;
    for (j = 0; j < mem_env; j++) {
      
      if (j < mem_env/2) offs = j; else offs = m->len + j;

      if (a[offs] != offs%120 && !m->reported) {
	printf("!!!BAD-MEM!!! -- [from %s %i] -- "
	       "curr_malloc=%i, i=%i, j=%i, offs=%i len=%li adr=%p ",
	       m->from, m->line,
	       curr_malloc, i, j, offs, m->len, m->adr);
	dump_mem(m);
	printf(" %i != %i ) \n", (int)a[offs], (int)offs);
	m->reported = 1;
	//abort();
      }
    }
  }
}

void dump_mem(MALL *m) {
  int i = 0;
  unsigned char *p = m->adr + mem_env/2;
  unsigned char c;

  while (i++ < m->len) {
    c = *p++;
    if (c < 32 || c > 127) c = '.';
    printf("%c", c);
  }

}



#  endif
#endif
