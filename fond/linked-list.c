/* a linked list implementation

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
#include "fond/allocate.h"
#include "fond/linked-list.h"
#include "fond/string.h"
#include "fond/object.h"

List *list_new(VALUE content) {
  List *l;
  l = alloc(List);
  l->class = class_list;
  l->head = content;
  l->rest = 0;
  return l;
}

List *list_new2(VALUE v1, VALUE v2) {
  List *l = list_new(v1);
  l->rest = list_new(v2);
  return l;
}

List *list_new3(VALUE v1, VALUE v2, VALUE v3) {
  List *l = list_new(v1);
  l->rest = list_new(v2);
  l->rest->rest = list_new(v3);
  return l;
}

List *list_new4(VALUE v1, VALUE v2, VALUE v3, VALUE v4) {
  List *l = list_new(v1);
  l->rest = list_new(v2);
  l->rest->rest = list_new(v3);
  l->rest->rest->rest = list_new(v4);
  return l;
}

void list_append(List **list, VALUE content) {
  List *new = list_new(content);
  if (*list == 0)
    *list = new;
  else
    list_last(*list)->rest = new;
}

void list_concat(List *list, List *l2) {
  list_last(list)->rest = l2;
}


int list_contains(List *l, VALUE v) {
  while (l) {
    if (l->head == v) return -1;
    l = l->rest;
  }
  return 0;
}

int list_find(List *l, VALUE v) {
  int i = 0;
  while (l) {
    if (l->head == v) return i;
    l = l->rest;
    i++;
  }
  return -1;
}

int list_length(List *l) {
  int  i = 0;
  while (l) {
    l = l->rest;
    i++;
  }
  return i;
}

List *list_last(List *l) {
  List   *lo = l;
  while  (l) {lo=l; l=l->rest;}
  return lo;
}



/* get the length of the longest path in the (sub-)tree *l  */
int list_depth(List *l) {
  int l1 = 1, l2 = 1;
  if (is_indirect(l->head) && !is_a_string(l->head)) {
    l1 = list_depth((List*)(l->head)) + 1;
  }
  if (l->rest != 0) {
    l2 = list_depth(l->rest);
  }
  return l1 > l2 ? l1 : l2;
}


void list_insert_sorted(List **pl, VALUE v) {
  List    *n, *l;

  n = list_new(v);
  if (*pl == 0) {
    *pl = n;
    return;
  }

  if (car(*pl) > v) {
    n->rest = *pl;
    *pl = n;
    return;
  }

  l = *pl;
  while (cdr(l) != 0 && cadr(l) < v)
    l = cdr(l);
  
  n->rest = cdr(l);
  l->rest = n;
}



//////////////////////
// ugly things...
VALUE list_walk(List *list, Iterator f) {
  List *l = list;
  VALUE r;

  do { r = (*f)(l); }
  while (r == IT_CONTINUE && (l=l->rest) != 0);

  return r;
}
/*int   list_find(List *list, VALUE content) {
  int idx = 0;
  VALUE vit(List *el) {
    if (el->head == content) { return idx; }
    else                     { idx++;  return IT_CONTINUE; }
  }  

  return list_walk(list, vit);
  }*/



///////////////////////////////////////////////////////


List* cdr(List *l)       { return l->rest; }
List* cddr(List *l)      { return cdr(cdr(l)); }
List* cdddr(List *l)     { return cdr(cddr(l)); }
List* cddddr(List *l)    { return cdr(cdddr(l)); }
List* cdddddr(List *l)   { return cdr(cddddr(l)); }
List* cddddddr(List *l)  { return cdr(cdddddr(l)); }

VALUE car(List *l)       { return l->head;    }
VALUE cadr(List *l)      { return car(cdr(l)); }
VALUE caddr(List *l)     { return car(cddr(l)); }
VALUE cadddr(List *l)    { return car(cdddr(l)); }
VALUE caddddr(List *l)   { return car(cddddr(l)); }
VALUE cadddddr(List *l)  { return car(cdddddr(l)); }

VALUE caar(List *l)      { return car((List*)car(l)); }
VALUE cadar(List *l)     { return cadr((List*)car(l)); }
VALUE caddar(List *l)    { return caddr((List*)car(l)); }
VALUE cadddar(List *l)   { return cadddr((List*)car(l)); }
VALUE caddddar(List *l)  { return caddddr((List*)car(l)); }

VALUE caadr(List *l)      { return car((List*)cadr(l)); }
List* cdar(List *l)       { return cdr((List*)car(l)); }
List* cdadr(List *l)      { return cdr((List*)cadr(l)); }
VALUE cadadr(List *l)     { return cadr((List*)cadr(l)); }


/*
  in:   List: ((k1 v1) (k2 v2) (k3 v3)), key: keyx
  out:  *res = vx
  return 0: found
*/
int list_assoc(List *map, VALUE key, List **res) {
  while (map) {

    if (!is_immediate(car(map))	&& ((List*)car(map))->class == class_list
	&& caar(map) == key)
    {
      assert(cdar(map) != 0);
      *res = cdar(map);
      return(0);
    }
    map = cdr(map);
  }
  *res = 0;
  return(1);
}

// return 0: found
int list_assoca(List *map, VALUE key, VALUE *res) {
  int    ret;
  List   *l;

  ret = list_assoc(map, key, &l);
  if (ret == 1) {
    *res = rb_nil;
    return ret;
  }

  *res = car(l);
  return ret;
}




#ifdef TEST_LL

#include <stdio.h>
#include <assert.h>

int n[] = {14,3,2,1,-20};

List
d    = { ptr(n), 0 },
  c    = {  ptr(n+1), &d },
    b    = {  ptr(n+2), &c },
      a    = {  ptr(n+3), &b },
	root = {  ptr(n+4), &a };

void test_walker(void);
void test_appender(void);
void test_finder(void);

int main() {
  assert( list_length(&root) == 5);
  assert( list_length(&d) == 1);
  assert( list_length(&c) == 2);

  assert( list_last(&c) == &d);
  assert( list_last(&root) == &d);

  test_walker();
  test_appender();
  test_finder();

  return(0);
}

void test_finder(void) {
  assert( list_find(&root, ptr(n+4)) == 0);
  assert( list_find(&root, ptr(n)) == 4);
  assert( list_find(&root, ptr(n+1)) == 3);

}

void test_walker(void) {
  int u = 0, sum = 0;

  VALUE vit(List *el) {
    assert(u < 5);
    assert(el->head != 0);
    sum += *iptr(el->head);
    u++;
    return IT_CONTINUE;
  }  

  list_walk(&root, vit);

  assert(u == 5);
  assert(sum == 0);
}


void test_appender(void) {
  List *li;
  int i = 123, j = 8;
  int l = list_length(&root);

  li = &root;

  list_append(&li, (VALUE)&i);
  assert( list_length(&root) == ++l);

  list_append(&li, (VALUE)&j);
  assert( list_length(&root) == ++l);

}

#endif
