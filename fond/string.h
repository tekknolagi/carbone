#if !defined(FOND_STRING_H)
#define FOND_STRING_H

#include "fond/rb.h"
#include "fond/value.h"

int is_a_string(VALUE v);
String *str_new(char *c);

extern Class *class_string;

#endif
