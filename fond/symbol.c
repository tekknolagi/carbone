/* fundamentals for class Symbol

  Copyright (C) 2002 Markus Liedl, markus.lado@gmx.de

  This file is part of Carbone.

  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
*/

#include <string.h>
#include <stdio.h>

#include "fond/allocate.h"
#include "symbol.h"
#include "linked-list.h"

List *Symbol_Table;             // global symbol table

// predefined symbols:

VALUE
   #define symbol_def(var, str) var,
   #include "fond/symbol_def.i"
   #undef symbol_def
                           not_a_symbol;


/*
  search the given string in the symbol-table
  returns: - rb_undef
        or - index of symbol as Sym
*/
Sym sym_lookup(char *str) {
  int idx = 0;
  Sym sym;
  List *l = Symbol_Table;

  while (l != 0) {
    if (strcmp((char*)l->head, str) == 0) {
      return(idx);
    }
    l = l->rest;
    idx++;
  }
  sym = rb_undef;

#if SYM_DEBUG
  printf("SYM_DEBUG:  (sym %i `%s')\n", sym, str);
#endif
  return(sym);
}

Sym sym_add(char *str) {
  Sym s;

  if ((s = sym_lookup(str))  ==  rb_undef) {
      char *ms = (char*)allocate(strlen(str)+1);
      strcpy(ms, str);
      list_last(Symbol_Table)->rest = list_new((VALUE)ms);
      return list_length(Symbol_Table)-1;
    }
    else
      return s;
}

VALUE intern(char *str){
  return idx2value(T_SYMBOL, sym_add(str));
}

char *sym_to_str(Sym idx) {
  List *l=Symbol_Table;
  while (l) {
    if (idx == 0)
      return((char*)l->head);
    l = l->rest;
    idx--;
  }
  return(0);
}


void sym_desc(VALUE sym) {
  printf("%s\n", sym2str(sym));
}

char *sym_sym2str(Sym s) {
  return sym2str(s);
}

Sym sym_str2sym(char *s) {
  return str2sym(s);
}

typedef struct init_symbols {
  VALUE *sym;
  char  *str;
} init_symbols;

static init_symbols symbols[] = {

#define symbol_def(var, str)   { &var, str},
#include "fond/symbol_def.i"
#undef symbol_def

};

void sym_init(void) {
  int i, num;
  
  Symbol_Table = list_new( (VALUE)"");

  num = sizeof(symbols) / sizeof(init_symbols);
  for(i = 0; i < num; i++) {
    *(symbols[i].sym) = intern(symbols[i].str);
  }
}






#ifdef TEST_SYMBOL
int main() {
  char  str[][10] = { "asd", "zui", "jkil", "uio", "+", "*", "/", "-", ":add", ":atom" };
  int   i, len = sizeof(str)/sizeof(str[0]);

  sym_init();

  for (i = 0; i < len; i++) {
    printf("inserting %s as %li\n", str[i], (long)sym_add(str[i]));
  }

  for (i = 0; i < len; i++) {
    printf("searching %s  --> got %li\n", str[i], (long)sym_add(str[i]));
  }

  for (i = 0; i < len; i++) {
    long idx = (long)sym_add(str[i]);
    printf("searching %s  --> got %li  --> str again: %s\n",
	   str[i],
	   idx,
	   sym_to_str(idx));
  }


  return(0);
}
#endif
