#if !defined(COMP_CFUNC_H)
#define COMP_CFUNC_H

#include "fond/symbol.h"
#include "fond/linked-list.h"


typedef struct cfunc_s {
  Sym  name;
  void *func;
  int  returns;
  int  arity;
} Cfunc;


void cfunc_declare(char *name, void* func, int returns, int arity);
Cfunc *cfunc_find(Sym name, int arity);


/* using a arity string instead of 'int arity' would allow to call func
   that take double arguments.

   numbers :      signify the numbers of long (4 byters)
   'l'     :      signify double arguments (long long,

   state:    not even thougt through
  */

extern List *cfunc_list;
void cfunc_init(void);
Cfunc *cfunc_new(char *name, void* func, int returns, int arity);


#endif
