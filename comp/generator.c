/* generate a sequence of VM instructions from input code

  Copyright (C) 2002 Markus Liedl, markus.lado@gmx.de
  Copyright (C) 2001 Free Software Foundation, Inc.

  This file is part of Carbone.
  Parts of this file are adopted from Gforth/vmgen-ex.

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


/*  see doc/generator.txt */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "configuration.h"

#include "fond/rb.h"
#include "fond/value.h"
#include "fond/allocate.h"
#include "fond/frame.h"
#include "vm/vm.h"
#include "vm/gen/rb-gen.i"
#include "vm/stack-layout.h"
#include "vm/cmcache.h"
#include "comm/lgram/lgram.h"
#include "comp/cfunc.h"
#include "comp/generator.h"
#include "fond/string.h"
#include "fond/code.h"
#include "fond/class.h"
#include "fond/method.h"
#include "fond/block.h"
#include "fond/object.h"
#include "fond/fixnum.h"
#include "fond/const.h"

List   *meth_walk_border;

void    generate(Inst **c, List *t, Callable *compilation_ctx);
void    gen_get_self(Inst **c, Callable *compilation_ctx);
void    gen_pick_own_frame(Inst **c, Callable *compilation_ctx);
void    gen_pick_outer_frame(Inst **c, Callable *compilation_ctx);
void    gen_access_loc(Inst **c, int rw,
		       int trgt, int nouter, int idx, Callable *ctx);
 
#define READ 1
#define WRITE 0

Inst   *vm_prim;
int     vm_debug;
int     vm_profile;

Inst   *vm_prim_nodebug, *vm_prim_debug;

extern Inst *vm_code_begin;

/* BB_BOUNDARY is needed on basic blocks without a preceding VM branch */
#define BB_BOUNDARY(c)\
     (last_compiled = NULL,  /* suppress peephole opt */ \
      block_insert(*c)) /* for accurate profiling */

/* later we may have better ways to flush the stack
   ((maybe without enforcing a superinstruction end)) */
#define FLUSH_STACK(x) BB_BOUNDARY(x)

int check_syntax(List *code);

/*
  Compile without giving a starting compilation context.
  Will be unused later, because there won't be code without context
 */
Code *gen_code(List *tree, int debug) {
  Inst *c;
  Code *code = code_new_and_block(debug);

  c = code->code;

  generate(&c, tree, 0);
  gen_end(&c);

  code->code_end = c;
  code_release_mempool(c);
  return(code);
}

Code *gen_code_callable(Callable *ca) {
  Code     *code;
  Inst     **c;

  code =  code_new_and_block(vm_debug);   // block code mempool
  code->code_end = code->code;
  c    =  (Inst**)&code->code_end;

  gen_check_params(c, ca);
  gen_alloc_lvars(c, ca);

  generate(c, ca->tree, ca);

  gen_drop_lvars(c, ca);
  gen_ret(c);
  gen_code_end(c);

  code_release_mempool(code->code_end);
  ca->code = code;
  gen_compile_deferred();

  return(code);
}

void generate(Inst **c, List *t, Callable *compilation_ctx) {
  List           *l;
  Cell           *patch_addr;

  /*-------------------------------------------- body ----*/
  if (car(t) == sym_body) {
    l = (List*)cadr(t);
    assert(l &&"empty bodys not yet suported");
    while(l) {
      generate(c, (List*)car(l), compilation_ctx);
      l = cdr(l);
      if (l)
	gen_drop(c);
    }
  }

  /*------------------------------------- lit and quote ----*/
  else if (car(t) == sym_lit || car(t) == sym_quote) {
    VALUE v = cadr(t);
    gen_lit(c, v);
    // note: v can be an arbitrary VALUE, so: immediate fixnum, imm. symbol
    // or reference to an arbitrary Object, but also List*
  }

  /*-------------------------------------------- char* ----*/
  // (char* "qweqwe")   generates a pointer to a char[]
  // (-deprecated-)
  // the lgram parser will later produce String objects and not char[]
  // from string literals; so this will drop out
  else if (car(t) == sym_char_ptr) {
      gen_lit(c, (VALUE)((String*)cadr(t))->string);
  }

  /*-------------------------------------------- const ----*/
  else if (car(t) == sym_const) {
    List  *ctx = (List*)cadr(t);
    VALUE cname = caddr(t);

    if ((VALUE)ctx == rb_nil) {
      if (const_exists(cname) == rb_true) {     // the constant is known:
	gen_lit(c, const_get(cname));
      } else {                       // the constant is still unknown:
	gen_lit(c, cname);
	gen_cfunc1(c, const_get);
	// gen_cfunc(... invalidate_self_code)
	// force recompile of this meth because now the const is known
      }
    } else {
      fprintf(stderr, "\ndifficult constants still unsupported\n");
      abort();
    }
  }
  /*-------------------------------------------- const= ----*/
  else if (car(t) == sym_const_asgn) {
    List  *ctx = (List*)cadr(t);
    VALUE cname = caddr(t);
    List  *asgn_to = (List*)cadddr(t);
    
    if ((VALUE)ctx == rb_nil) {
      gen_lit(c, cname);
      generate(c, asgn_to, compilation_ctx);
      gen_cfunc2(c, const_set);
      // notify (e.g. recompile) dependent code; const_set() should do that
    } else {
      fprintf(stderr, "\ndifficult constants still unsupported\n");
      abort();
    }
  }
  /*-------------------------------------------- eql ----*/
  else if (car(t) == sym_eql) {
    List *a = (List*)cadr(t);
    List *b = (List*)caddr(t);
    generate(c, a, compilation_ctx);
    generate(c, b, compilation_ctx);
    gen_eql(c);
  }

  /*-------------------------------------------- scond ----*/
  else if (car(t) == sym_scond) {
    VALUE kind = cadr(t);
    List *p = (List*)caddr(t);
    List *q = (List*)cadddr(t);

    #if 0
    if (car(t) == sym_scond)      kind = cadr(t);
    else if (car(t) == sym_and)   kind = rb_true;
    else if (car(t) == sym_or)    kind = rb_false;
    #endif

    generate(c, p, compilation_ctx);
    gen_dup(c);
    kind == rb_true ? gen_braf(c, 0) : gen_brat(c, 0);
    patch_addr = (*(Cell**)c) - 1;
    gen_drop(c);
    generate(c, q, compilation_ctx);
    *patch_addr = *(Cell*)c; BB_BOUNDARY(c);
  }

  /*-------------------------------------------- long_if ----*/
  else if (car(t) == sym_long_if) {
    List *lop;
    Cell **patch_eoi;   // where to put the addr of End-Of-If
    int p, pc = 0;
    lop = (List*)cadr(t);  // list of pairs (cond,then)
    patch_eoi = allocn(list_length(lop), Cell*);
    while(lop) {
      List *cond = (List*)caar(lop);
      List *then = (List*)cadar(lop);
      generate(c, cond, compilation_ctx);
      gen_braf(c, 0);    // braf after_then
      patch_addr = (*(Cell**)c) - 1;
      generate(c, then, compilation_ctx);
      gen_jmp(c, 0);      // braf after_if
      patch_eoi[pc++] = (*(Cell**)c) - 1;
      *patch_addr = *(Cell*)c;  // patch after_then
      lop = lop->rest;
    }
    // generate else:
    generate(c, ((List*)caddr(t)), compilation_ctx );
    BB_BOUNDARY(c);
    p = -1;    // patch branches to 'end of if'
    while(++p < pc) *(patch_eoi[p]) = *(Cell*)c;
  }


  /*-------------------------------------------- while ----*/
  else if (car(t) == sym_while) {
    List *cond = (List*)cadr(t);
    List *loop = (List*)caddr(t);
    Inst *l_while;
    Cell **pa_test_cond;               // pa == patch-adress
    gen_lit(c, rb_false); 
    gen_jmp(c, 0);                     // jmp test_cond

      pa_test_cond = (*(Cell***)c) - 1;
    l_while = *c;
    generate(c, loop, compilation_ctx);
    gen_nip(c); 

      *pa_test_cond = *(Cell**)c;  BB_BOUNDARY(c);
    generate(c, cond, compilation_ctx);
    gen_brat(c, l_while);
    /* diff-to-matz: the value of the last evaluation of the body
                     is returned */
  }

  /*-------------------------------------------- cfunc ----*/
  else if (car(t) == sym_cfunc) {
    Sym     name;
    int     arity;
    Cfunc   *f;
    List    *params;

    name = cadr(t);
    arity = list_length(t) - 2;
    params = cddr(t);

    f = cfunc_find(name, arity);
    if (arity > 0) {
      while (params) {
	generate(c, (List*)car(params), compilation_ctx);  // the parameters ...
	params = cdr(params);
      }
    }
    if (f->returns) {
      // function returns VALUE
           if(arity == 0 ) gen_cfunc0(c, f->func);
      else if(arity == 1 ) gen_cfunc1(c, f->func);
      else if(arity == 2 ) gen_cfunc2(c, f->func);
      else if(arity == 3 ) gen_cfunc3(c, f->func);
      else abort();
    }
    else {
      // void function
           if(arity == 0 ) gen_cfuncv0(c, f->func);
      else if(arity == 1 ) gen_cfuncv1(c, f->func);
      else if(arity == 2 ) gen_cfuncv2(c, f->func);
      else abort();
      gen_lit(c, rb_nil);  // [FIXME]
    }
  }

  /*-------------------------------------------- send ----
      (send
        rcv
        sel
        args
        [rest_args]
        [proc])
  */
  /*    parameters could be also optional */
  else if (car(t) == sym_send || car(t) == sym_m) {
    List* rcv       = (List*) cadr(t);
    VALUE sel       =         caddr(t);
    List* args      = (List*) cadddr(t), *l;
    VALUE rest_args =         rb_nil;
    List* proc      = (List*) rb_nil;
    int   num_args, i, dup_params_flag = 0;


    /* rest_args and proc are optional */
    l = cddddr(t);
    if (l != 0) {
      rest_args = car(l);
      if (rest_args != rb_nil)  {
	/* [FIXME] does not work correctly if rest_param_array does
	   not come from immediately enclosing context */
	if (compilation_ctx->rest_param_array == rest_args)
	  dup_params_flag = 1;
      }
      if (cdr(l) != 0)
	proc = (List*) cadr(l);
    }

    generate(c, rcv, compilation_ctx);    // rcv

    num_args = 0;
    l = args;                             // gen arguments
    if ( (VALUE)l != rb_nil) {
      while(l) {
	generate(c, (List*)car(l), compilation_ctx);
	gen_swap(c);                      // keep rcv on top
	l = cdr(l);
	num_args++;
      }
    }

    if(dup_params_flag) gen_dupParams(c);

    gen_lit(c, sym2idx(sel));             // selector (as Sym)

    gen_lit(c, num_args);                 // num_args

    if(dup_params_flag) {
      gen_pick(c, stk_num_param_offs());  // num of params of enclosing meth
      gen_add(c);
    }

    if ((VALUE)proc != rb_nil) {
      // push fp and proc on proc-stack
      // generate(c, proc, compilation_ctx); // ??, ptr to proc
    }

    gen_send(c);

    if(!dup_params_flag) {                  // drop arguments
      for (i = 0; i < num_args; i++) {
	gen_nip(c);
      }
    } else {
      gen_pick(c, stk_num_param_offs());
      gen_nipn(c);
    }
    /* (What gets actually dropped are the first num_args-1 arguments and self.
       The methods result becomes the new top of stack.) */
  }
  /* We evaluated the rcv before the arguments (or it wouldnt be Ruby)
     and we have to give the rcv after the parameters, so
     currently I do a swap after each parameter, to keep the rcv on
     top.  [FIXME]. 

     It is necessary to eval the rcv before the parameters only if the
     former evaluation has side effects. */


  /*------------------------------------ invoke a Method/Block -----*/
  /* the difference to `send' is, that the Method is not looked up
     at runtime, but evaluated by the code compiled from caddr.
     (But support for dynamic selectors is better implemented with
      `send') 

     It is assumed that the Method is one that is compiled for
     the receivers class.

     [FIXME] parameters missing
     [FIXME] there are no checks whether meth or block is compiled to
             _really work_ with the given receiver
  */
  else if (car(t) == sym_invoke) {
    // [push parameters]
    generate(c, (List*)cadr(t), compilation_ctx);   // rcv
    generate(c, (List*)caddr(t), compilation_ctx);  // meth or block
    gen_lit(c, 0);                                  // num of params
    gen_invoke(c);
    // [drop parameters]
  }


  /* what does `in method context' mean?

     We can (1) during compilation get the method that encloses this
     code via the `compilation_ctx', and we can (2) generate code
     that accesses (parameters, rcv, num-parameters) on the
     stack. They are there because a `send' or `invoke' was executed,
     that left them in a certain order on the stack (written down
     in  vm/stack-layout.h). 
  */

  /*----------------------------- in method/proc context: self -----*/
  else if (car(t) == sym_self) {
    gen_get_self(c, compilation_ctx);
  }

  /*----------------------------- in method/proc context: variables ----*/
  else if (car(t) == sym_lv || car(t) == sym_lv_asgn) {

    VALUE  varname;
    int    ret, nouter, idx;
    varname = cadr(t);

    ret = ca_find_lvar(compilation_ctx, varname, &nouter, &idx);

    assert(ret != 0);

    if (car(t) == sym_lv_asgn) {
      List    *asgn_to = (List*)caddr(t);
      generate(c, asgn_to, compilation_ctx);
    }

    gen_access_loc(c, (car(t) == sym_lv ? READ : WRITE),
		   ret, nouter, idx, compilation_ctx);
  }

  /*----------------------------- in method/proc context: parameters ----*/
  /*
    [FIXME] optional params
   */
  else if (car(t) == sym_par || car(t) == sym_par_asgn) {

    VALUE  parname;
    int    ret, nouter, idx;
    parname = cadr(t);

    ret = ca_find_param(compilation_ctx, parname, &nouter, &idx);

    assert(ret != 0);


    if (car(t) == sym_par_asgn) {
      List    *asgn_to = (List*)caddr(t);
      generate(c, asgn_to, compilation_ctx);
    }

    gen_access_loc(c, (car(t) == sym_par ? READ : WRITE),
		   ret, nouter, idx, compilation_ctx);
  }

  /*------------------------ in method/block context: ivar ----*/
  else if (car(t) == sym_iv || car(t) == sym_iv_asgn) {
    Callable         *s;
    Class            *ocls, *cls;
    long             pos, offs;

    // [FIXME] ivars of mixins not yet covered

    s = compilation_ctx;
    while (s != 0  &&  s->class != class_method) {
      s = (Callable*) s->outer;
    }
    assert(s);
    assert(s->outer == 0);
    assert(s->class == class_method);

    ocls = cls = ((Method*)s)->recv_class;

    do {
      pos = list_find(cls->ivars.names, cadr(t));
      if (pos >= 0) goto found_ivar;
      cls = cls->super;
    } while(pos == -1 && cls != 0);
  found_ivar:

    if (pos == -1) {
      printf("ivar %s not found for class %s,\n",
	     sym_sym2str(cadr(t)), sym_sym2str(ocls->name));
      abort(); // [[FIXME]]
    }
    offs  = class_ivar_offset(cls, pos); 

    if (car(t) == sym_iv) {
      gen_get_self(c, compilation_ctx);
      gen_apick(c, offs);
    }
    else if (car(t) == sym_iv_asgn) {
      List    *asgn_to = (List*)caddr(t);
      generate(c, asgn_to, compilation_ctx);
      gen_get_self(c, compilation_ctx);
      gen_astab(c, offs);
    }
  }

  /*----------------------------- in method context: num_param --*/
  else if (car(t) == sym_num_param) {
    gen_pick(c, stk_num_param_offs());
  }
  /*------------------------------- in method context: proc_given -----*/
  else if (car(t) == sym_proc_given) {
    // gen_pick(c, FP_PROC);
    // see on proc-stack whether the sender has pushed a proc
    assert(0);
  }
  /*-------------------------------- in method/proc context: ret -----*/
  else if (car(t) == sym_ret) {
    assert(0 && "[FIXME] ret is buggy!");
    generate(c, (List*)cadr(t), compilation_ctx);
    gen_ret(c);
  }
  /*------------------------------------------------ boundp -----*/
  else if (car(t) == sym_boundp) {
    generate(c, (List*)cadr(t), compilation_ctx);  // class
    generate(c, (List*)caddr(t), compilation_ctx); // sel
    gen_cfunc2(c, class_boundp);
  }

  /*------------------------------------------------ class -----*/
  else if (car(t) == sym_class_of) {
    generate(c, (List*)cadr(t), compilation_ctx);  // object
    gen_classOf(c);
  }

  /*-------------- in method or block context: proc creation -----*/
  else if (car(t) == sym_block) {
    /*
    parameters given here
    ---------------------
    Names of 
    * params
    * rest-array-params
    * svars
    * lvars

    Value of
    * dont-wrap-one-arg
            (1) `proc{|*a| }'      <===>    (2) `proc{|a| }'
            Dont wrap a single param in an array
            The compiler translates all (2) to (1) and sets this flag
            if he does.
            ('proc{|a|a}[1]' ==>1;  but 'proc{|a|a}[1,2]' ==>[1, 2])


    */

    Block   *blk;
    List    *proc_def;
    
    proc_def = (List*) cdr(t);

    blk = blk_new(proc_def, compilation_ctx);
    
    gen_lit(c, (VALUE)blk);
    gen_cf1(c, (void*)blk_new_proc); 
    /*
      cf1 is a C funtion call that passes in the frame pointer, so
      the Proc initializer blk_new_proc() can suck out the current
      activation frame. 
    */

    
    gen_add_compile_deferred( (Object*) blk);



	/*

    generated code (runtime; VM code)
    ---------------------------------

    * compile Block code (lazily, so only first time when it's not yet
                          compiled) 
    * create a Proc object
      store in there:
        * self
        * all in Block.outer_contexts remarked 
    */
    

  }


  else {
    puts("FATAL ERROR in generate.c: unknown tree node");
    printf("generate: t = %lx,   car(t) = %lx,  cdr(t) = %lx\n",
	   (Cell)t, (Cell)car(t), (Cell)cdr(t));
    tree_dump(t);
    abort();
  }
}


// generate code that checks the number of parameters
// [FIXME]  correct for block num_args == 0 ==> no check!
// [FIXME]  correct for rest-args...
// [FIXME]  respect optional parameters (for methods)
void gen_check_params(Inst **c, Callable *ca) {
  
  if (ca->rest_param_array == rb_nil) {
    gen_chkpar(c, ca->params.length);
  }
  else {                          // there is a rest-array
    if (ca->params.length > 0)
      gen_chkparMin(c, ca->params.length);  // check only for Minimum #parameters
    else
      ;                             // no check
  }
}

/*
  There are two cases:
  (1) has_frame == false
  (2) has_frame == true

  Then we do:
  * allocate memory for lvars (1) on stack or (2) create a Frame
    object.

  * prepare the pointer to the Frame object on the stack.
    It points to (1) the outer Frame or (2) the current Frame.
*/

void gen_alloc_lvars(Inst **c, Callable *ca) {
  int flush = 0;
  if (!ca->has_frame) {
    if (ca->outer) {
      assert(ca->class == class_block);
      gen_pick(c, stk_rcv_offs());
      gen_apick(c, offsetof(Proc, outer_frame)); // leaves outer Frame* on stack
      flush = 1;
    }

    if (ca->lvars.length < MAX_NUM_SVARS_COMP_AS_LIT) {
      int i = 1;
      if (ca->lvars.length >= 1) {
	gen_lit(c, rb_undef);
	while (i++ < ca->lvars.length)
	  gen_dup(c);
	flush = 1;
      }
    }
    else {
      gen_lit(c, ca->lvars.length);
      gen_allot(c);
      flush = 1;
    }
  }
  else {  // has_frame == true
    gen_cf0(c, frm_create);    // ==> frm_create(current_fp);
    flush = 1;                 //     leaves Frame* on stack
  }

  if (flush)
    FLUSH_STACK(c);
}


/*
  generate code that drops all the lvars of this Callable;

  this means
    1. drop stack variables, or
    2. drop ref. to Frame

  uses nip to keep the Callables result
*/

void gen_drop_lvars(Inst **c, Callable *ca) {

  if (!ca->has_frame) {
    if (ca->lvars.length < MAX_NUM_SVARS_COMP_AS_LIT) {
      int i = 0;
      while (i++ < ca->lvars.length) gen_nip(c);
    }
    else {
      gen_lit(c, ca->lvars.length);
      gen_nipn(c);
    }
    if (ca->outer)
      gen_nip(c);
  }
  else {  // has_frame == true
    gen_nip(c);
  }
}

void gen_get_self(Inst **c, Callable *compilation_ctx) {
  if (compilation_ctx->class == class_method) {
    gen_pick(c, stk_rcv_offs());
  } else if (compilation_ctx->class == class_block) {

    if (compilation_ctx->has_frame)
      gen_pick_own_frame(c, compilation_ctx);
    else
      gen_pick_outer_frame(c, compilation_ctx);

    gen_apick(c, frm_offs_self());
  } else
    assert(0);
}


/* 
   for Callables that have an own Frame
 */
void gen_pick_own_frame(Inst **c, Callable *compilation_ctx) {
  assert(compilation_ctx->has_frame);
  gen_pick(c, stk_frame_offs());
}

/* 
   for Callables that have no own Frame
 */
void gen_pick_outer_frame(Inst **c, Callable *compilation_ctx) {
  assert(!compilation_ctx->has_frame);
  assert(compilation_ctx->outer->has_frame);
  gen_pick(c, stk_frame_offs());
}


void gen_code_end(Inst **end_of_code) {
  assert(*end_of_code < ((Inst*)vm_code_begin+CODE_LENGTH_MAX) &&
         "produced too much code; increment CODE_LENGTH_MAX in fond/code.h");
}


void gen_access_loc(Inst **c, int read,
		    int trgt, int nouter, int idx,
		    Callable *ctx) {
  int offs;

  offs = idx*sizeof(Cell);

  if (trgt == STACK) {
    if (read)
      gen_pick(c, offs);
    else
      gen_stab(c, offs);
  }
  else if (trgt == FRAME) {
    if (nouter == 0) {
      gen_pick_own_frame(c, ctx);
    } else {
      if (ctx->has_frame) {
	gen_pick_own_frame(c, ctx);
      } else {
	gen_pick_outer_frame(c, ctx);
	nouter--;
      }
      if (nouter > 0)
	gen_apick(c, frm_offs_outer(nouter, ctx));
    }
    if (read)
      gen_apick(c, offs);
    else
      gen_astab(c, offs);
  }
  else assert("wrong trgt" && 0);
}






int generator_initialized = 0;

void gen_init(void) {
  {if (generator_initialized) return;} generator_initialized = 1;

#if defined(ENGINE_DEBUG)
  (void)engine_debug(NULL,NULL,NULL,NULL); /* initialize vm_prim */
  vm_prim_debug = vm_prim;
#endif

  (void)engine(NULL,NULL,NULL,NULL); /* initialize vm_prim */
  vm_prim_nodebug = vm_prim;

  if (vm_debug) vm_prim = vm_prim_debug;
  init_peeptable();

  // meth_walk_border = list_new3(sym_lit, sym_quote, sym_block);
  meth_walk_border = list_new2(sym_lit, sym_quote);
}



List *uncompiled_objects = 0;         // [[FIXME]] global var

void gen_add_compile_deferred(Object *blk) {

  list_append(&uncompiled_objects, (VALUE)blk);
  
}

void gen_compile_deferred(void) {
  Object *o;

  while (uncompiled_objects != 0) {
    o = (Object*) car(uncompiled_objects);
    uncompiled_objects = cdr(uncompiled_objects);

    if (o->class == class_block) {
      Callable *ca = (Callable*)o;
      gen_code_callable(ca);

      if (vm_debug) {
	printf("\nxxxxxxxxxxxxxxxxxxxxxxxx code for block %p\n", ca);
	tree_dump(ca->tree);
	code_dump_stdout(ca->code);
      }
    }
    else
      assert(0 && "should only be Blocks; what do you want?");
  }
}

