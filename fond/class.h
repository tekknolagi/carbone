extern long counter_class_find_method;

void     class_init(void);

Class*   class_of_imm(VALUE v);
Class*   class_new(VALUE name, Class *super);
void     class_additional_boot_setup(Class *class);
Class*   class_define(VALUE classname, VALUE supercls, List *clsdef);
Method*  class_compile_body(Class *class, List *classbody);

long     class_ivar_offset(Class *class, long ivar_pos);
int      class_size(Class *c);
int      class_number_ivar(Class *c);
int      class_base_offs(Class *c);

VALUE    class_is_ancestor(Class *c, Class *other);
Class*   class_of(VALUE v);

Method*  class_find_method(Class *c, Sym selector);
void     class_dump_selectors(Class *c);
VALUE    class_boundp(Class *c, VALUE sel);

void     class_append_method(Class *c, Method *meth);

void     dispatch_error(VALUE self);

#define m_class_of(rcv,res) \
  if (is_indirect(rcv))    res = ((Object*)rcv)->class; \
                   else    res = m_class_of_imm(rcv)

