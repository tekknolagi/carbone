/* fundamentals and initialisation for the Ruby object system

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

#include <stdlib.h>
#include "fond/value.h"
#include "fond/rb.h"
#include "fond/allocate.h"

#include "fond/class.h"
#include "fond/object.h"
#include "fond/fixnum.h"
#include "fond/const.h"

#include "comp/cfunc.h"

Class *class_object;
Class *class_module;
Class *class_class;
Class *module_kernel;

Class *class_code;
Class *class_method;
Class *class_block;
Class *class_proc;

Class *class_fixnum;
Class *class_symbol;
Class *class_constant;

Class *class_list;

Class* ary_imm_class[NUM_IMM_VALUES];


void obj_init() {

  class_object = alloc(Class);
  class_object->name = intern("Object");

  class_module = alloc(Class);
  class_module->name = intern("Module");

  module_kernel = alloc(Class);
  module_kernel->name = intern("Kernel");

  class_class = alloc(Class);
  class_class->name = intern("Class");

  class_object->super  = NULL;
  class_module->super  = class_object;
  module_kernel->super = NULL;
  class_class->super   = class_module;

  class_object->mixins  = list_new( (VALUE)module_kernel);
  class_module->mixins  = NULL;
  module_kernel->mixins = NULL;
  class_class->mixins   = NULL;

  class_object->class  = class_class;
  class_module->class  = class_class;
  module_kernel->class = class_module;
  class_class->class   = class_class;

  class_additional_boot_setup(class_object);
  class_additional_boot_setup(class_module);
  class_additional_boot_setup(module_kernel);
  class_additional_boot_setup(class_class);

  const_set(intern("Object"), (VALUE)class_object);
  const_set(intern("Module"), (VALUE)class_module);
  const_set(intern("Kernel"), (VALUE)module_kernel);
  const_set(intern("Class"),  (VALUE)class_class);


  class_code     =  class_new(intern("Code"), class_object);
  class_method   =  class_new(intern("Method"), class_object);
  class_block    =  class_new(intern("Block"), class_object);
  class_proc     =  class_new(intern("Proc"), class_object);

  class_fixnum   =  class_new(intern("Fixnum"), class_object);
  class_symbol   =  class_new(intern("Symbol"), class_object);
  class_constant =  class_new(intern("Constant"), class_object);

  ary_imm_class[T_FIXNUM] = class_fixnum;
  ary_imm_class[T_SYMBOL] = class_symbol;
  ary_imm_class[T_CONSTANT] = class_constant;

  class_list =  class_new(intern("List"), class_object);

  cfunc_declare("obj_new", obj_new, 1, 1);
}


VALUE obj_new(Class *c) {
  Object *obj;

  obj = allocate(class_size(c) * sizeof(VALUE));
  obj->class = c;

  return(VALUE)obj;
}

VALUE obj_instance_of(Object *obj, Class *c) {
  return class_is_ancestor(obj->class, c);
}



//// obsolete: List is now a full object with OBJECT_HEADER!

// this function isn't used that much ... and shouldn't
VALUE is_a_object(VALUE v) {
  Object *o;
  if (is_indirect(v)) {
    o = (Object*)v;
    if (is_indirect(o->class) &&

	o->class->class == class_class &&      // <- instead of this
	/*is_indirect(o->class->class) &&       ... those would be more correct
	  class_is_ancestor(o->class->class, class_class) &&                    */
	obj_instance_of(o, class_object) == rb_true)
      {
	return rb_true;
      }
  }
  return rb_false;
}

