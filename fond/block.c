/* Block 

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
#include <stdlib.h>
#include "fond/value.h"
#include "fond/rb.h"
#include "fond/linked-list.h"
#include "fond/linked-list-tree.h"
#include "fond/code.h"
#include "fond/class.h"
#include "fond/method.h"
#include "fond/callable.h"
#include "comp/cfunc.h"
#include "comp/generator.h"
#include "vm/vm.h"
#include "vm/stack-layout.h"



void   blk_examine_callback(List *c, Object *user_param);
VALUE  prc_pick_self(char *fp, Block *b);
Proc*  blk_new_proc(char* fp, Block *b);

List   *block_lex_border;
Method *blk_meth_call, *blk_meth_slc;

void blk_init(void) {
  char  *call = "call",
        *slc  = "[]";

  cfunc_declare("blk_new_proc", blk_new_proc, 1, 2);
  meth_from_builtin(class_proc, call, "ProcCall");
  meth_from_builtin(class_proc, slc, "ProcCall");

  blk_meth_call = class_find_method(class_proc, sym2idx(intern(call)));
  blk_meth_slc  = class_find_method(class_proc, sym2idx(intern(slc)));
}



/* ------------------------------------------------------
   blk_new()

   create a new Block object at compile time

---------------------------------------------------------*/
Block *blk_new(List *proc_def, Callable *comp_ctx) {
  Block *b;
  List  *sign, *param, *lvar, *tree;
  VALUE rest, dont_wrap_single;
  int   has_frame;

  b                      = alloc(Block);
  b->class               = class_block;
  b->outer               = comp_ctx;

  sign = (List*) car(proc_def);
  tree = (List*) cadr(proc_def);

  if ( (VALUE)sign != rb_nil) {
    list_assoc(sign, sym_param,  &param);
    list_assoc(sign, sym_lvar,   &lvar);

    list_assoca(sign, sym_rest, &rest);
    list_assoca(sign, sym_dont_wrap_single, &dont_wrap_single);
    if (dont_wrap_single == rb_nil)
      dont_wrap_single = rb_false;
    has_frame = list_contains(sign, sym_heap_frame);
  }
  else {
    param = lvar = 0;
    rest = rb_nil;
    dont_wrap_single = rb_false;
    has_frame = 0;
  }


  ca_initialize((Callable*) b, comp_ctx, param, rest, lvar, has_frame, tree);

  b->dont_wrap_single    = dont_wrap_single;

  return(b);
}




/*
  blk_new_proc()

  Create a new Proc object. This method is called directly from the
  VM.

  fp is the frame pointer to the currently active frame, which can be
  a method or a proc, but must have a corresponding Frame in the heap.
*/

Proc *blk_new_proc(char* fp, Block *b) {
  Proc  *p;

  p = alloc(Proc);

  p->class       = class_proc;
  p->block       = b;
  p->frame       = 0;
  p->outer_frame = stk_frame(fp); 

  return(p);
}

