#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "comm/lgram/lgram.h"

extern FILE *yyin;
YYSTYPE result;

int yyparse(void);
void lgram_init(void);

List *lgram_read_file(char *filename)
{
  FILE *f;
  List *l;

  if ( (f=fopen(filename,"r")) == NULL) {
    perror(filename);
    abort();
  }

  l = lgram_read_stream(f);

  fclose(f);
  return(l);
}


void *lex_set_input(FILE *i);

List *lgram_read_stream(FILE *f) {
  void *p;
  int r;
  lgram_init();
  yyin = f;
  p = lex_set_input(yyin);

  r = yyparse();
  
  yy_delete_buffer(p);
  if (r == 0)
    return( (List*)(result.list)->head );
  else
    return(0);
}


List *lgram_read_string(char *str) {
  lgram_init();
  yy_scan_string(str);
  if (yyparse() == 0)
    return( (List*)(result.list)->head );
  else
    return(0);
}

void *lex_set_input(FILE *i) {
  void *p;
  yy_switch_to_buffer(p=yy_create_buffer( i, 4096));
  return(p);
}



Sym sym_true, sym_false, sym_nil;


/*-----------------------------------------------------*/
/* internalize lgram datatypes, e.g.                   */
/*   * (List*)'(false)'  ==> (VALUE)rb_false           */
/*-----------------------------------------------------*/
List *lgram_element(List *l) {
  if ( l->rest == 0) {
    if (l->head == sym_true)   l = (List*)rb_true;
    else if (l->head == sym_false)  l = (List*)rb_false;
    else if (l->head == sym_nil)    l = (List*)rb_nil;
  }
  return(list_new((VALUE)l));
}

int lgram_initialized  = 0;

void lgram_init() {
  if (lgram_initialized) return;
  sym_true = intern("true");
  sym_false = intern("false");
  sym_nil = intern("nil");
}
