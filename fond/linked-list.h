
#if !defined(LINKED_LIST_H)
#define LINKED_LIST_H

#include "fond/value.h"
#include "fond/object_header.h"

typedef struct List
{
  OBJECT_HEADER
  VALUE        head;
  struct List  *rest;
} List;

List*     list_new(VALUE content);

void      list_append(List **list, VALUE content);
void      list_insert_sorted(List **pl, VALUE v);
void      list_concat(List *list, List *l2);
int       list_contains(List *l, VALUE v);  // gives -1 or 0
int       list_find(List *l, VALUE v);      // gives index or -1 if failing

List*     list_last(List *l);
int       list_length(List *list);
int       list_depth(List *l);

List*     list_new2(VALUE v1, VALUE v2);
List*     list_new3(VALUE v1, VALUE v2, VALUE v3);
List*     list_new4(VALUE v1, VALUE v2, VALUE v3, VALUE v4);
int       list_assoc(List *map, VALUE key, List **res);
int       list_assoca(List *map, VALUE key, VALUE *res);

#if !defined(LL_INLINE_CADR)

List* cdr(List *l);
List* cddr(List *l);
List* cdddr(List *l);
List* cddddr(List *l);
List* cdddddr(List *l);
List* cddddddr(List *l);

VALUE car(List *l);
VALUE cadr(List *l);
VALUE caddr(List *l);
VALUE cadddr(List *l);
VALUE caddddr(List *l);
VALUE cadddddr(List *l);

VALUE caar(List *l);
VALUE cadar(List *l);
VALUE caddar(List *l);
VALUE cadddar(List *l);
VALUE caddddar(List *l);

VALUE caadr(List *l);
List* cdadr(List *l);
VALUE cadadr(List *l);

#else
static inline List* cdr(List *l)       { return l->rest; }
static inline List* cddr(List *l)      { return cdr(cdr(l)); }
static inline List* cdddr(List *l)     { return cdr(cddr(l)); }
static inline List* cddddr(List *l)    { return cdr(cdddr(l)); }
static inline List* cdddddr(List *l)   { return cdr(cddddr(l)); }
static inline List* cddddddr(List *l)  { return cdr(cdddddr(l)); }

static inline VALUE car(List *l)       { return l->head;    }
static inline VALUE cadr(List *l)      { return car(cdr(l)); }
static inline VALUE caddr(List *l)     { return car(cddr(l)); }
static inline VALUE cadddr(List *l)    { return car(cdddr(l)); }
static inline VALUE caddddr(List *l)   { return car(cddddr(l)); }
static inline VALUE cadddddr(List *l)  { return car(cdddddr(l)); }

static inline VALUE caar(List *l)      { return car((List*)car(l)); }
static inline VALUE cadar(List *l)     { return cadr((List*)car(l)); }
static inline VALUE caddar(List *l)    { return caddr((List*)car(l)); }
static inline VALUE cadddar(List *l)   { return cadddr((List*)car(l)); }
static inline VALUE caddddara(List *l)  { return caddddr((List*)car(l)); }

static inline VALUE caadr(List *l)      { return car((List*)cadr(l)); }
static inline List* cdadr(List *l)      { return cdr((List*)cadr(l)); }
static inline VALUE cadadr(List *l)     { return cadr((List*)cadr(l)); }
#endif

#include "fond/symbol.h"

// ...
// kill this iterator stuff??
typedef VALUE (*Iterator)(List *el);
VALUE     list_walk(List *list, Iterator fnct);



#endif

