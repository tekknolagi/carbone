// boehm-tree: an arbitrary datastructure (tree) using the boehm collector


#include <stdio.h>
#include <time.h>

//#define NO_GC

#ifdef NO_GC
#  include <malloc.h>
#  define gc_malloc(res,x) (res=malloc(x))
#  define message "no gc\n"
#else
#  if !defined(INL)
#    include <gc/gc.h>
#    define gc_malloc(res,x) (res=GC_MALLOC(x))
#    define message "gc\n"
#  else
#    include <gc/gc_inline.h>
#    define gc_malloc(res,x)  GC_MALLOC_WORDS(res,x/sizeof(int))
#    define message "gc/inl\n"
#  endif
#endif

#define tree_depth 13
#define repetition 200

typedef struct TreeStruct {
  struct TreeStruct *a;
  struct TreeStruct *b;

#ifndef EXTRA_BYTES
# define EXTRA_BYTES 0
#endif
  char extra[EXTRA_BYTES];

} Tree;


void iter(int, Tree*);
void cut(Tree *t, int l);

int main() {
  int     i, j;
  Tree*   root[repetition];
  clock_t t1, t2;
  float   res;
  int     num_obj, mem_per_iter;

  num_obj = 1 << tree_depth;
  mem_per_iter = sizeof(Tree)*num_obj;

  setbuf(stdout, 0);
  printf("allocating %i * %ikB   obj-size=%i\n",
	 repetition,
	 mem_per_iter / 0x400,
	 sizeof(Tree));
  printf(message);

  t1 = clock();
  for(i = 0; i < repetition; i++) {
    gc_malloc(root[i], sizeof(Tree));
    iter(tree_depth, root[i]);

    #ifdef CUT
    cut(root[i], 0);

      #ifdef FREE_LEVEL
      if (i >= FREE_LEVEL) root[i-FREE_LEVEL] = NULL;
      #endif
    #endif

    printf(".");
  }
  t2 = clock();
  res = (float)((t2-t1+0e-10)/CLOCKS_PER_SEC);
  printf("\nspent %1.5f sec (%3.5f MBytes/s   %3.3f KObj/s) \n",
	 res,
	 mem_per_iter*repetition/res/(1<<20),
	 num_obj*repetition/res/(1<<10));
	 
	 
}

void iter(int d, Tree *t) {
  Tree *a, *b;
  if (d > 0) {
    gc_malloc(a, sizeof(Tree));
    t->a = a;
    iter(d-1, a);
    gc_malloc(b, sizeof(Tree));
    t->b = b;
    iter(d-1, b);
  } else {
    t->a = (void*)0x12345;
    t->b = (void*)0x54321;
  }
}

void cut(Tree *t, int d) {
  Tree *a, *b;

  static int ca,cb,cc,cd;
  if (d == 0) {
    ca = cb = cc = cd = 0; 
  }

  if (d == 2 && ca++ % 2 == 0  ||
      d == 3 && cb++ % 2 == 0  ||
      d == 4 && cc++ % 2 == 0  ||
      d == 5 && cd++ % 2 == 0  ||
      d == 6)
  {
    t->a = t->b = 0;  // free
  }

  if (t->a != NULL && (void*)t->a > (void*)0xfffff)
    cut(t->a, ++d);

  if (t->b != NULL && (void*)t->b > (void*)0xfffff)
    cut(t->b, ++d);
}
