/* wrapper for compiled code

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
#include "fond/code.h"

extern int vm_debug;
extern int vm_profile;
extern FILE* vm_out;


#define STACK_SIZE 20000

extern Inst *vm_prim_debug;
extern Inst *vm_prim_nodebug;
extern int vm_profile;

Inst *vm_code_begin = (Inst*)0, *vm_code = (Inst*)0;

void code_init() {
  if (vm_code == (Inst*)0)
    vm_code = vm_code_begin = allocate(CODE_LENGTH_MAX * sizeof(Cell));
}

int code_mempool_blocked = 0;

/*
  generate new Code object

  you have to give the debug flag when initializing this object and
  you can't change it later; you have to create a new Code object.

  warning:
  the debug flag is currently ignored, because changing between the
  two engines imposes other unsolved problems.
 */
Code *code_new_and_block(int use_debug_engine) {
  Code *code;

  assert(!code_mempool_blocked &&
	 "no two methods can be compiled at the same time");

  code_mempool_blocked = 1;   // [[FIXME]] global var


  code = alloc(Code);

  code->class = class_code;

#if defined(ENGINE_DEBUG)
  code->engine = use_debug_engine ? engine_debug : engine;
#else
  code->engine = engine;
#endif

  code->prim = vm_prim;
  code->code = vm_code;
  code->insn = 0;

  return(code);
}

void code_release_mempool(Inst *till) {

  vm_code = till;

  // vm/profile.c needs the end of generated code.
  // but this is bad code...
  // the whole mempool construct...
  vmcode_end =   (till > vmcode_end) ? till : vmcode_end;

  code_mempool_blocked = 0;
}

void code_dump_stdout(Code *c) {
  FILE *old;

  old = vm_out;
  vm_out = stdout;

  vm_disassemble(c->code, c->code_end, c->prim);

  vm_out = old;
}

VALUE code_invoke(Code *code, int trace, int profile) {
  Cell *stack, *s;
  VALUE res;
  engine_t vm;


  stack    = allocn(STACK_SIZE, Cell);
  
  vm_debug = trace;
  vm = (engine_t)code->engine;
  vm_out = stdout;
  
  //if (profile)
  //  init_peeptable();

  s = stack + STACK_SIZE-1;
  res = vm(code->code,
	   s /* sp */,
	   0 /* fp */,
           0 /* procsp */);

  return(res);
}

void code_dump_profile() {
  // vm_prim = vm_prim_debug;
  vm_print_profile(stderr);
}
