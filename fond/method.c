/* fundamentals for class Method

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


/*

  Matz' Ruby Method objects are like proc{}'s since they contain also
  the receiver.

  carbone Method objects don't know this receiver, so you can't call a
  Method instance without giving a value for self and you can't
  convert a Method instance to a Proc instance without giving a value
  for self.

  Should I rename my Method Class??

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fond/value.h"
#include "fond/rb.h"
#include "fond/method.h"
#include "fond/linked-list.h"
#include "fond/code.h"
#include "fond/class.h"
#include "fond/callable.h"
#include "comp/cfunc.h"
#include "comp/generator.h"
#include "vm/stack-layout.h"
#include "vm/vm.h"

void meth_init() {
  cfunc_declare("meth_generate", meth_generate, 1, 2);
  cfunc_declare("meth_from_list", meth_from_list, 1, 2);
}

/*
  define a method
  assuming this Method is defined the first time.


    (meth_name
     ((param a b c)
      (opt   (f (lit 10))
             (i (const (nil) AAA)))
      (lvar  ab)
      (proc  ab)
      (rest  ya))
     (body  (( .... ))))     */

Method *meth_from_list(Class *c, List *meth_def) {
  Method *meth;
  List   *sign, *tree, *param, *lvar;
  VALUE  proc, rest;
  int    has_frame;

  assert("see if method is good formated: " &&
	 meth_def != 0
	 && cdr(meth_def) != 0
	 && cadr(meth_def) != 0
	 && cddr(meth_def) != 0);

  assert("see if given class is really a class: " &&
	 c != 0 && c->class == class_class);         // [FIXME] Modules

  meth = alloc(Method);
  meth->class = class_method;
  meth->recv_class = c;
  meth->name = car(meth_def);
  sign = (List*)cadr(meth_def);
  tree = (List*)caddr(meth_def);

  if ((VALUE)sign != rb_nil) {
    list_assoc(sign, sym_param,  &param);
    list_assoc(sign, sym_lvar,   &lvar);
    list_assoc(sign, sym_opt,    &meth->optionals);

    list_assoca(sign, sym_proc, &proc);
    list_assoca(sign, sym_rest, &rest);
    has_frame = list_contains(sign, sym_heap_frame);
  }
  else {
    param = lvar = meth->optionals = 0;
    proc = rest = rb_nil;
    has_frame = 0;
  }

  ca_initialize((Callable*)meth,
		0, param, rest, lvar, has_frame, tree);

  class_append_method(c, meth);

  // [FIXME]s
  // invalidate lots of compiled code, if
  // * new method contains new attributes (for a superclass?!!!)
  // [* new method overwrites a somewhere inlined method,...]

  return meth;
}



Code *meth_generate(Method *m, int debug) {
  gen_code_callable( (Callable*)m );
  if (debug) {
    printf("\nxxxxxxxxxxxxxxxxxxxxxxxx code for method %s#%s\n",
	   sym_sym2str(m->recv_class->name),
	   sym_sym2str(m->name));
    code_dump_stdout(m->code);
  }
  return m->code;
}

void meth_from_builtin(Class *class, char *sel_str, char *insn_name) {
  VALUE  sel;
  Method *p;
  Code   *code;

  sel = sym_str2sym(sel_str);

  p = alloc(Method);
  p->class            = class_method;
  p->recv_class       = class;
  p->name             = sel;
  p->params.names     = list_new(intern("other"));
  p->params.length    = 1;
  p->rest_param_array = rb_nil;
  p->proc             = rb_nil;
  p->lvars.names      = 0;
  p->lvars.length     = 0;
  p->tree             = 0;
  
  code = alloc(Code);
  code->class = class_code;
  p->code = code;

#if defined(ENGINE_DEBUG)
  code->engine = vm_debug ? engine_debug : engine; // ??
#else
  code->engine = engine; // ??
#endif

  code->prim = vm_prim;
  code->code = 0;
  code->insn = vmdesc_get_insn_from_sym(intern(insn_name));

  class_append_method(class, p);
}

void meth_report_wrong_num_parameters(VALUE rcv, char *meth, VALUE num_par) {
  Class *class = class_of(rcv);
  fprintf(stderr,
	  "%s#%s called with wrong number (%li) of parameters",
	  sym2str(class->name),
	  meth,
	  num_par);
}

void meth_report_wrong_num_parameters2(Inst *ip, Cell *sp, char *fp) {
  VALUE rcv    = (VALUE) stk_rcv(fp);
  int num_par  = stk_num_param(fp);
  meth_report_wrong_num_parameters(rcv, "<unknown>", num_par);
}

void meth_report_dyn_recomp_unsupp(VALUE rcv, VALUE sel) {
  fprintf(stderr,
	  "FATAL ERROR: dynamic recompilation still unsupported\n"
	  "             rcv: %p  rcv.class: %p  sel: %s\n",
	  (void*)rcv, class_of(rcv), sym2str(sel));
  abort();
}

void meth_report_does_not_exist(VALUE rcv, VALUE sel) {
  fprintf(stderr, "ERROR: method `%s' does not exist for "
	          "rcv %p (%s)\n",
	  sym2str(sel),
	  (void*)rcv,
	  sym2str(class_of(rcv)->name));
  // [FIXME]
  abort();
}
