/* generic c function calls

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

#include <stdio.h>
#include <assert.h>
#include "fond/allocate.h"
#include "stdlib.h"
#include "fond/value.h"
#include "fond/rb.h"
#include "string.h"
#include "comp/cfunc.h"
#include "fond/class.h"
#include "fond/linked-list-tree.h"

List *cfunc_list = 0;

/*

  There is a serious portability issue, if C functions that are used here
  have non-integer parameters, other than VALUE (which is a long).
  On the Alpha processor, integer parameters are handed over in integer
  registers.

 */


void cfunc_init() {
  list_append(&cfunc_list,
	      (VALUE)cfunc_new("printf", &printf, 0, 2) );

  list_append(&cfunc_list, (VALUE)cfunc_new("tree_dump", &tree_dump, 0, 1));
  list_append(&cfunc_list, (VALUE)cfunc_new("value2long", &value2long, 1, 1));

  // those are not intended for serious use;
  // only for testing (see tests/t_cfunc.lg)
  list_append(&cfunc_list, (VALUE)cfunc_new("printf", &printf, 0, 1));
  list_append(&cfunc_list, (VALUE)cfunc_new("malloc", &malloc, 1, 1));
  list_append(&cfunc_list, (VALUE)cfunc_new("strcpy", &strcpy, 1, 2));
  list_append(&cfunc_list, (VALUE)cfunc_new("strcat", &strcat, 1, 2));
}

Cfunc *cfunc_new(char *name, void* func, int returns, int arity) {
  Cfunc *f;

  f = alloc(Cfunc);
  f->name    = intern(name);
  f->func    = func;
  f->returns = returns;
  f->arity   = arity;

  return f;
}

Cfunc *cfunc_find(Sym name, int arity) {
  List *l =  cfunc_list;
  while (l != 0) {
    Cfunc *f = (Cfunc*)l->head;
    if (f->name == name && f->arity == arity) return f;
    l = l->rest;
  }
  fprintf(stderr, "unknown C function %s/%i\n", sym2str(name), arity);
  return 0;
}

void cfunc_declare(char *name, void* func, int returns, int arity) {
  list_append(&cfunc_list, (VALUE)cfunc_new(name, func, returns, arity));
}


