/* offer VM primitives as Symbols to the running VM

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


#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "fond/rb.h"
#include "fond/allocate.h"
#include "vm/vm.h"

char insns_as_strings[][200] = {
#include "gen/rb-desc.i"
};

VALUE *insns_as_symbols;

int number_of_insns;

void init_vm_desc() {
  int i;

  number_of_insns = sizeof(insns_as_strings)/sizeof(insns_as_strings[0]);
  insns_as_symbols = allocn(number_of_insns, VALUE);
  
  for (i = 0; i < number_of_insns; i++) {
    assert(strlen(insns_as_strings[i]) < sizeof(insns_as_strings[0]));
    insns_as_symbols[i] = intern(insns_as_strings[i]);
  }
}


Inst* vmdesc_get_insn_from_sym(VALUE sym) {
  int i;
  for (i = 0; i < number_of_insns; i++)
    if (insns_as_symbols[i] == sym)
      return(vm_prim[i]);

  fprintf(stderr, "unknown instruction %s", sym_sym2str(sym));
  abort();
}
