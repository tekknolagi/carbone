/* functions that treat linked lists as trees

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
#include <string.h>
#include "fond/value.h"
#include "fond/allocate.h"
#include "fond/linked-list-tree.h"
#include "fond/symbol.h"
#include "fond/object.h"
#include "fond/string.h"
#include "fond/value.h"

/*
  tree_depth and tree_dump are regarding lists
  as a tree. list->rest are all brothers to the right 
             list->head is child/ or value
*/



char *list_dump0(List *, int ind, int no_ind, int tol_in);
void tree_dump_imm(char *b, VALUE v);
void drin(char *b, int x) {int i; for(i = 0; i < x; i++) strcat(b, "  ");}


void tree_dump(List *l) {
  char *c = list_dump0(l, 0, 1, 1);
  if (c[strlen(c)-1] == ')') c[strlen(c)-1] = 0;
  puts (c);
}


#define linesize 80
char* list_dump0(List *l, int ind, int no_indent, int tol_in) {
  int  toln, tol; /* tol: try one line */
  char *b, *tr1 = NULL, *tr2 = NULL;
  
  b = allocate(linesize+5000);

  toln = tol_in;
  if (list_depth(l) > 4) toln = 0;
  while(1) {
    tol = toln; toln = 0; b[0] = 0;

#define check(b) if (tol&&strlen(b)>linesize) continue;

    if (!tol && !no_indent) { drin(b, ind); check(b); }

    //
    // sprintf(b+strlen(b), " [%i %i%i|", ind, tol, no_indent);
    //

    check(b);

    if (is_indirect(l->head) && !is_a_string(l->head)) {
      if(!tol) { strcat(b, " "); }    strcat(b, "("); check(b);
      if (tr1 == NULL) tr1 = list_dump0( (List*)l->head, ind+1, 1, 1);

#define check2(b,c) if (tol&&strlen(b)+strlen(c)>linesize) continue;

      check2(tr1,b);
      strcat(b, tr1);
    } else {
      tree_dump_imm(b, l->head);
      check(b);
    }

    if (l->rest != 0) {
      if (!tol) strcat(b,"\n"); else strcat(b, " ");
      check(b);
      // if (tr2 == NULL) // enabling this if gives pretty long runtime
        tr2 = list_dump0(l->rest, ind, 0, tol);
      check2(tr2,b);
      strcat(b, tr2);
    } else {
      strcat(b, ")");
    }
    check(b);
    break;
  }
  return(b);
}

// should dump value in l->head
// string and other objects are exceptions to rule.
void tree_dump_imm(char *b, VALUE v) {
  if (v == rb_true) strcat(b, " (true)");
  else if (v == rb_false) strcat(b, " (false)");
  else if (v == rb_nil) strcat(b, " (nil)"); 
  else if (is_a_string(v)) {
    String *s = (String*)v;
      strcat(b, "\"");
      strcat(b, s->string);
      strcat(b, "\"");
  }
  else if (is_a_object(v) == rb_true) {
    Object *o = (Object*)v;
    sprintf(b+strlen(b), "#<%s:0xlx", sym2str(o->class->name));
  }
  else {
    if (is_fixnum(v))   sprintf(b+strlen(b), "%li", value2long(v)); else
      if (is_symbol(v))   sprintf(b+strlen(b), "%s", sym2str(v));
    return;
  }
}



#ifdef TEST_LLT

#include <stdio.h>
#include <assert.h>

int main() {
  List *t, *l = list_new(113);
  int i;

  t = l;
  for (i = 1; i < 20; i++) {
    if (i%5 != 0) {
      t->rest = (List*)list_new(i);
      t = t->rest;
    } else {
      t->head = (VALUE)list_new(i);
      t = (List*)t->head;
    }
  }
  tree_dump(l);

  return(0);
}

#endif
