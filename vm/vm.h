/* definitions for Carbone

  Copyright (C) 2002 Markus Liedl, markus.lado@gmx.de
  Copyright (C) 2001 Free Software Foundation, Inc.

  This file is part of Carbone.
  This file was part of Gforth.

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

#if !defined(VMG_RB_H)
#define VMG_RB_H

#include <stdio.h>
#include "fond/value.h"
#include "fond/rb.h"

typedef void *Label;
typedef void *Inst; /* for direct threading, the same as Label */
typedef long Cell;


#define USE_FP 1

#if defined(USE_FP)
#define if_FP(x)
#endif

extern Inst *vm_prim;
extern int locals;
extern Cell peeptable;
extern int use_super;
extern int vm_debug;
extern char *program_name;
extern FILE *vm_out;
extern Inst *vmcodep;
extern Inst *last_compiled;
extern Inst *vmcode_end;

/* generic vmgen support functions (e.g., wrappers) */
void gen_inst(Inst **vmcodepp, Inst i);
void init_peeptable(void);
void vm_disassemble(Inst *ip, Inst *endp, Inst prim[]);
void vm_count_block(Inst *ip);
struct block_count *block_insert(Inst *ip);
void vm_print_profile(FILE *file);

/* for the debug engine: print instructions when tracing */
void trace_insn(char *name, Inst *_ip, char *fp, Cell *sp);

/* type-specific support functions */
void genarg_i(Inst **vmcodepp, Cell i);
void genarg_target(Inst **vmcodepp, Inst *target);
void genarg_cfunc(Inst **vmcodepp, void *f);
void printarg_i(Cell i);
void printarg_target(Inst *target);
void printarg_cfunc(void *ary);
void printarg_sel(Sym sel);
void printarg_meth(Method *m);
void printarg_class(Class *m);
void printarg_rcv(VALUE rcv);

#define printarg_Cell(i)  printarg_i((Cell)i)
#define printarg_oldfp(i) printarg_i((Cell)i)
#define printarg_proc(i)  printarg_i((Cell)i)
#define printarg_retip(i) printarg_i((Cell)i)
#define printarg_ptr(i)   printarg_i((Cell)i)
#define printarg_other(i)   printarg_i((Cell)i)

/* engine functions (type not fixed) */
Cell engine(Inst *ip0, Cell *sp, char *fp, Cell *procsp);
Cell engine_debug(Inst *ip0, Cell *sp, char *fp, Cell *procsp);

typedef Cell (*engine_t)(Inst *ip0, Cell* sp, char* fp, Cell *procs);

/* get vm instruction via its name as a string; desc.c */
void init_vm_desc(void);
Inst* vmdesc_get_insn_from_sym(VALUE sym);


static inline int in_imm_range(long xxx)  {
  return (((unsigned long)((long)xxx)+value_imm_type_max/2) <=
	  ((unsigned long)(value_imm_type_max)));
}
#endif
