/* fundamentals for class Class

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
#include "fond/rb.h"
#include "fond/linked-list-tree.h"
#include "main.h"
#include "fond/value.h"
#include "fond/allocate.h"
#include "fond/method.h"
#include "fond/class.h"
#include "fond/object.h"
#include "fond/const.h"
#include "fond/callable.h"
#include "fond/code.h"
#include "comp/cfunc.h"
#include "comp/generator.h"
#include "vm/vm.h"   // for vm_debug
#include "vm/cmcache.h"

long counter_class_find_method = 0;

void class_init() {
  cfunc_declare("class_define", class_define, 1, 3);
  cfunc_declare("class_compile_body", class_compile_body, 1, 2);
  cfunc_declare("class_is_ancestor", class_is_ancestor, 1, 2);
  cfunc_declare("class_of", class_of, 1, 1);
  cfunc_declare("class_boundp", class_boundp, 1, 2);
}

Class* class_new(VALUE name, Class *super) {
  Class *class;

  class = alloc(Class);

  if (super == 0)
    super = class_object;

  assert(is_symbol(name));

  class->class         = class_class;
  class->name          = name;
  class->super         = super;
  class->method_names  = 0;
  class->methods       = 0;

  class_additional_boot_setup(class);

  const_set(name, (VALUE)class);

  return class;
}


void class_additional_boot_setup(Class *class) {
  class->all_ivars       = -1;
  class->inst_compl_size = -1;
}


// not yet class re-define-capable

Class *class_define(VALUE classname, VALUE supercls, List *clsdef) {
  Class *c, *super;
  List  *sign, *ivar, *cvar, *lvar;
  int i;

  assert("see if class def is good formated"
	 && clsdef != 0
	 && cdr(clsdef) != 0);

  super = (Class*)supercls;
  if (super != 0 && super->class != class_class) {
    fprintf(stderr, "ERROR: Object given as superclass is not a class\n");
    return(0);
  }

  c = class_new(classname, super);
  sign    = (List*) car(clsdef);
  c->body = (List*) cadr(clsdef);

  if ((VALUE)sign != rb_nil) {
    list_assoc(sign, sym_lvar,  &c->lvars.names);
    list_assoc(sign, sym_ivar,  &c->ivars.names);
    list_assoc(sign, sym_cvar,  &c->cvars.names);
  }
  else {
    ivar = cvar = lvar = 0;
  }
  c->ivars.length = list_length(c->ivars.names);
  c->cvars.length = list_length(c->cvars.names);
  c->lvars.length = list_length(c->lvars.names);
  
  c->cvar_values = allocn(c->cvars.length, VALUE); 
  for(i = 0; i < c->cvars.length; c->cvar_values[i++] = rb_undef);

  if (vm_debug)
    printf("\nclass_new(%s<%s) class_size(%s)=%i   class_size(%s)=%i\n",
	   sym_sym2str(c->name), sym_sym2str(super->name),
	   sym_sym2str(super->name), class_size(super),
	   sym_sym2str(c->name), class_size(c));

  return(c);
}

// returned Method object is not a real Method; it contains nothing
// but the code for the class def; It does not get registered as a
// method in class->method_names nor in class->methods.
Method *class_compile_body(Class *c, List *classbody) {

  // variable contexts inside a class def are similar to those in
  // methods, so we are using the same compiling function as for
  // methods.

  Method *m;
  int    has_frame;

  c->body = (List*)cadr(classbody); // [[FIXME]] multiple class bodies?

  if (vm_debug) { printf("\ncompiling class body:\n"); tree_dump(c->body); }

  m = alloc(Method);

  m->class               = class_method;
  m->recv_class          = c;
  m->name                = intern("intern-class-def-meth");

  has_frame = 1;

  ca_initialize( (Callable*)m, 0, 0, rb_nil, c->lvars.names, has_frame, c->body);

  m->proc                = rb_nil;

  gen_code_callable( (Callable*)m );

  // [FIXME] class body is not interesting: make extra enabler for dump?
  if (vm_debug) code_dump_stdout(m->code);

  // return the resulting method, so it can be invoked
  return(m);
}


VALUE class_is_ancestor(Class *c, Class *other) {
  while(c) {
    if (c == other || list_contains(c->mixins, (VALUE)other))
	return rb_true;
    c = c->super;
  }
  return rb_false;
}


Class* class_of(VALUE v) {
  Class *c;
  m_class_of(v,c);
  return c;
}



Class *class_of_imm(VALUE v) {
  switch(i_type(v)) {
  case T_SYMBOL:    return(class_symbol);
  case T_FIXNUM:    return(class_fixnum);
  case T_CONSTANT:  return(class_constant);  //[FIXME] True, False, Nil !
  }
  fprintf(stderr,
	  "ERROR in class.c#class_of_imm(): unknown immediate VALUE 0x%lx!\n",
	  v);
  return (Class*) 0;
}

#if defined(DBG_SHOW_METH_LOOKUP_PATH)
  #define DBG_SMLP(class) if (vm_debug) class_dump_selectors(class)
#else
  #define DBG_SMLP(class) 
#endif



// used in every send that is not cached.
// no :missing lookup!
Method *class_find_method(Class *c, Sym selector) {
  List*   mix;
  int     idx;

  counter_class_find_method++;

  while(c) {
    DBG_SMLP(c);

    // look for method in class
    idx = list_find(c->method_names, selector);
    if (idx >= 0)
      return c->methods[idx];

    mix = c->mixins;
    while(mix) {
      // look for method in module
      Class *mixin = ((Class*) car(mix));
      DBG_SMLP(mixin);

      idx = list_find(mixin->method_names, selector);
      if ( idx >= 0)
	return ((Class*)mix->head)->methods[idx];
      mix = cdr(mix);
    }

    // same procedure for superclass
    c = c->super;
  }

  return (Method*)0;
}

VALUE class_boundp(Class *class, VALUE sel) {
  Method *meth;
  Sym    sym = sym2idx(sel);

  lookup_method(class, sym, meth);

  return meth == 0  ? rb_false : rb_true;
}

void class_dump_selectors(Class *c) {
  List *l;
  printf("\n\t\t\t\t<%s{ ", sym2str(c->name));
  l = c->method_names;
  while(l!=0) {
    printf("%s", sym2str(car(l)));
    l = cdr(l);
    if (l!=0) printf(" ");
  }
  printf("}>\n\t\t\t");
}

/* instances memory layout

Carbone has a linear memory layout; that means: each instance keeps
all its state (the instance variables) in exactly one linear block of
memory. It's organized in a way that methods of a superclass can act
on instances of subclasses without depending on their
internals. (surely the superclass will only act on ivars it knows of)

ivars of Modules involve one more step at runtime: One and the same
Module can be included in multiple classes; but each class can also
include multiple Modules, so each Module has to ask it's mixee where it
got included  (where==which offset); this can't be performed at
runtime when on doesn't want to compile specialized methods (one for
each class it got mixed in)


class_size(class) is used to get the number of ivars for a given class 


,------------------------------
| C struct                      \ 
|   (Class *class; ...           } class_base_offs()
|    SomeStruct *some_struct;)  /
|-----------------------------   -\                 -\ 
| superclass                       |                  |
| superclass mixin 1                >  class_size(    |
| superclass mixin 2               |     superclass)  |
| ...                              |                  |
|-----------------------------   -/                    > class_size(
| subclass                                            |       subclass)
| subclass mixin 1                                    |
| subclass mixin 2                                    |
| ...                                                 |
|-----------------------------                      -/
| subsubclass
| subsubclass mixin 1
| subsubclass mixin 2
| ...
`-----------------------------

  to give an example:

assuming Class subsubclass < subclass < superclass < Object;
 and each class has 2 mixins
 and each class and each mixin introduce one ivar;
we`d get:
                       byte-offsets (if sizeof(VALUE) == 4)
Class *class                   0x00
superclass      @iv1           0x04
superclass mx1  @iv2           0x08
superclass mx2  @iv3           0x0c
subclass        @iv4           0x10
subclass mx1    @iv5           0x14
subclass mx2    @iv6           0x18
subsubclass     @iv7           0x1c
subsubclass mx1 @iv8           0x20
subsubclass mx2 @iv9           0x24
                                   */


long class_ivar_offset(Class *class, long ivar_pos) {
  int super_size;
  int offs;

  assert(class != class_object);

  super_size = class_size(class->super);   /* C-struct + all supers */

  offs = super_size + ivar_pos;

  return     offs*sizeof(VALUE);
}

int class_size(Class *c) {                // used for every instantiation
  int base, ivars, s;

  if (c->inst_compl_size != -1)
    return (c->inst_compl_size);

  base = class_base_offs(c);
  ivars = class_number_ivar(c);

  s = c->inst_compl_size = base + ivars;
  
  #if defined(DBG_SHOW_CLASS_SIZE)
    if (vm_debug)
      {
	printf("\nclass_size(%s)=%i  (base=%i,ivars=%i)\n",
	       sym2str(c->name), s, base, ivars);
      }
  #endif

  return (s);
}



int class_number_ivar(Class *c) {
  int n = 0;
  List *m;

  if (c == 0) return(0);
  if (c->all_ivars != -1) return (c->all_ivars);

  if (c->ivars.names) {
    n += c->ivars.length;
  }

  m = c->mixins;
  while(m) {
    if (((Class*)car(m))->ivars.names)
      n += ((Class*)car(m))->ivars.length;
    m = m->rest;
  }

  return (c->all_ivars = n + class_number_ivar(c->super));
}


#define num_VALUES(strct) (sizeof(strct)/sizeof(VALUE))

/*
  all builtin classes (those that have C structs) are direct
  subclasses of Object (except Class, which is equal to Module (in
  terms of instance size))
*/

int class_base_offs(Class *c) {
  int   n = num_VALUES(Object);

  while (c->super && c->super != class_object) {
    c = c->super;
  }

       if (c == class_module) n = num_VALUES(Class);   // covers also class_class
  else if (c == class_method) n = num_VALUES(Method);
  else if (c == class_code)   n = num_VALUES(Code);
  else if (c == class_block)  n = num_VALUES(Block);
  else if (c == class_proc)   n = num_VALUES(Proc);
  else if (c == class_list)   n = num_VALUES(List);

//else if (c == class_array)  n = num_VALUES(Array);
// ...

  return n;
}

void class_append_method(Class *c, Method *meth) {
  int    num_meth;
  num_meth = list_length(c->method_names);
  c->methods = realloc( c->methods, (num_meth+1) * sizeof(Method*));

  list_append(&c->method_names, sym2idx(meth->name));

  c->methods[num_meth] = meth;
}



void dispatch_error(VALUE self) {
  fprintf(stderr,
	  "dispatch_error(self=0x%lx), self.class=%s\n",
	  self,
	  sym2str(class_of(self)->name)
	  );
}
