/* fundamentals for class String

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

#include "fond/value.h"
#include "fond/rb.h"
#include "fond/allocate.h"
#include <string.h>

Class *class_string = (Class*)0x89786745;

String *str_new(char *c) {
  String *s = alloc(String);
  s->class = class_string;
  s->string = string_dup(c);
  return s;
}

int is_a_string(VALUE v) {
  if (is_indirect(v)) {
    String *s = (String*)v;
    if (s->class == class_string) return(-1); else return(0);
  }
  return(0);
}

char* get_char_ptr(String *s) {
  return(s->string);
}
