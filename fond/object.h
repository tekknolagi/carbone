
#if !defined(OBJECT_H)
#define OBJECT_H

#include "fond/allocate.h"
#include "fond/rb.h"
#include "fond/value.h"

extern Class *class_object;
extern Class *class_module;
extern Class *class_class;
extern Class *module_kernel;

extern Class *class_code;
extern Class *class_method;
extern Class *class_block;
extern Class *class_proc;

extern Class *class_fixnum;
extern Class *class_symbol;
extern Class *class_constant;

extern Class *class_list;

extern Class *ary_imm_class[NUM_IMM_VALUES];

void obj_init(void);

VALUE  obj_new(Class *c);
VALUE  obj_instance_of(Object *obj, Class *c);
VALUE  is_a_object(VALUE v);


#endif
