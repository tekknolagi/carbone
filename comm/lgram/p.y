%{
#include <stdlib.h>
#define YYDEBUG 1

#define token_already_included
#include "comm/lgram/lgram.h"
#include "configuration.h"

void yyerror(char *s) { fprintf (stderr, "%s\n", s); }
int yywrap(void)      { return 1; }
List *lgram_element(List *l); // in lgram.c

#if defined(LGRAM_DEBUG_PARSER)
 #include <stdio.h>
 #define YYDEBUG 1 
 //int yydebug;
 yydebug = 1;
#endif 

%}

%union {
  struct List *list;
  VALUE       value;
  char        *str;
}

%{
extern YYSTYPE result;
%}

%type  <list>   forms form elements element
%token <str>    NUM IDENT STRING



%%

forms       : form
		{
		  result.list = $$ = $1;
		}
            | forms form
		{
                  list_concat(result.list = $$ = $1, $2);
		}
	    ;


form	    : '(' elements ')'
		{
		  $$ = lgram_element($2);
		}
            | '(' ')'
                {
		  $$ = rb_nil;
                }
	    ;


elements    : element
	    | elements element
		{
                  list_concat($$=$1, $2);
		}
	    ;


element     : NUM
		{
		  long val = strtol($1, NULL, 10);
		  assert_can_be_fixnum( abs(val) );
		  $$ = list_new( long2value(val));
		}
	    | IDENT
		{
		  $$ = list_new( str2sym($1));
		}
	    | STRING
		{
		  $$ = list_new( (VALUE)str_new($1));
		}
            | form
	    ;
%%


