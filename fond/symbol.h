
#if !defined(SYMBOL_H)
#define SYMBOL_H

#include "fond/linked-list.h"

typedef long Sym;
/* Sym is the index of a Symbol. It is not a VALUE!

   the macros sym2str() and str2sym() in value.h work actually
   on VALUEs (Sym packaged as VALUEs)

   the methods here expect Sym's i.e. indizes
*/

Sym   sym_lookup(char *str);
Sym   sym_add(char *str);
VALUE intern(char *str);
char  *sym_to_str(Sym idx);
char  *sym_sym2str(Sym s);
Sym   sym_str2sym(char *s);
void  sym_desc(VALUE sym);
void  sym_init(void);


extern VALUE
   #define symbol_def(var, str) var,
   #include "fond/symbol_def.i"
   #undef symbol_def
                           not_a_symbol;


#endif
