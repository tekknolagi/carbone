/* wrapper for compiled fixnum builtins

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

#define MAY_BE_INLINED inline static
#include "fond/rb.h"
#include "fond/allocate.h"

#include "fond/class.h"
#include "fond/object.h"
#include "fond/method.h"
#include "comp/cfunc.h"

#include "value.c"
#include "fond/value.h"

#include "vm/builtins.h"

VALUE  sym_plus,  sym_minus, sym_mul, sym_div, sym_lt, sym_gt,
  sym_cmp, sym_uplus, sym_uminus;

void fixnum_init(void) {

  sym_plus = intern("+");
  sym_minus = intern("-");
  sym_mul = intern("*");
  sym_div = intern("/");
  sym_lt = intern("<");
  sym_gt = intern(">");
  sym_cmp = intern("<=>");
  sym_uplus = intern("@+");
  sym_uminus = intern("@-");

#include "../builtins/fixnum_bi_decl.i"

}

VALUE fixnum_overflow(VALUE operation, VALUE rcv, VALUE other) {

  fprintf(stderr,
	  "fixnum_overflow(%s, 0x%lx, 0x%lx)\n",
	  sym_sym2str(operation), rcv, other);

  return (rb_undef);
}

VALUE fixnum_coerce(VALUE operation, VALUE rcv, VALUE other) {

  fprintf(stderr,
	  "fixnum_coerce(%s, 0x%lx, 0x%lx)\n",
	  sym_sym2str(operation), rcv, other);

  return (rb_undef);
}
