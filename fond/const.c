/* base constants

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
#include "fond/rb.h"
#include "fond/value.h"
#include "fond/const.h"
#include "comp/cfunc.h"

List *constants;

void const_init() {
  cfunc_declare("const_get", const_get, 1, 1);
  cfunc_declare("const_set", const_set, 1, 2);

#if TEST_CONSTANTS
  const_set(intern("Abc"), 123);
  const_set(intern("A"), 1);
  const_set(intern("B"), 2);
  const_set(intern("C"), 3);
  assert(const_get(intern("Abc")) == 123);
  assert(const_get(intern("A")) == 1);
  assert(const_get(intern("B")) == 2);
  assert(const_get(intern("C")) == 3);
#endif

}

VALUE const_exists(VALUE name) {
  List *l = constants;
  List *c;

  while (l) {
    c = (List*)car(l);
    if (car(c) == name) return(rb_true);
    l = (List*)cdr(l);
  }
  return(rb_false);
}


VALUE const_get(VALUE name) {
  List *l = constants;
  List *c;
  while (l) {
    c = (List*)car(l);
    if (car(c) == name) return cadr(c);
    l = (List*)cdr(l);
  }
  fprintf(stderr,
	  "reading unset constant %s!\n",
	  sym2str(name));

  assert("lazy const compilation still unsupported" && 0);
  return(rb_nil);
}

VALUE const_set(VALUE name, VALUE val) {
  char b = sym2str(name)[0];
  List *l = constants;
  List *c;

  if (b < 'A' || b > 'Z')
    fprintf(stderr,
            "syntax error: constant is not capitalized (`%s')\n",
	    sym2str(name));

  while (l) {
    c = (List*)car(l);
    if (car(c) == name) break;
    l = (List*)cdr(l);
  }
  if (l == 0)
    list_append(&constants, (VALUE)list_new2(name, val));
  else {
    fprintf(stderr,
	    "warning: already initialized constant `%s'\n",
	    sym2str(name));

    // invalidate depending code...
    assert("recompilation is still unsupported" && 0);

    cdr(l)->head = val;
  }

  // printf("constants:\n");
  // tree_dump(constants);

  return(val);
}
