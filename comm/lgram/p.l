%{
#include <stdio.h>
#include <string.h>
#include "fond/allocate.h"
#include "comm/lgram/lgram.h"

%}

%option noyywrap

DIGIT		[0-9]
IDENT_CHAR	[a-zA-Z_@+*/!$%&|`~,.:\-=<|>?\[\]]

%%

-?{DIGIT}+	{ yylval.str = yytext;
                  return NUM;
                }

{IDENT_CHAR}(({IDENT_CHAR}|[0-9])*) {
                  yylval.str = yytext;
                  return IDENT;
                }


\"([^"\\]|\\(\\|\"|n|t))*\" {
		  char *str, *res = allocate(strlen(yytext)+1);
		  char *in = yytext + 1;
		  str = res;
		  while (*in) {
		    if (*in == '\\') {
		           if (in[1] == '\\')  {*str = '\\'; in+=2;}
		      else if (in[1] == '"')   {*str = '"'; in+=2;}
		      else if (in[1] == 'n')   {*str = '\n'; in+=2;}
		      else if (in[1] == 't')   {*str = '\t'; in+=2;}
		      else { fprintf(stderr,
                                     "false backslash in string `%s'\n", yytext); }
		    } else {
		      *str = *in; in++;
		    }
		    str++;
		  }
		  *(str-1) = 0;
		  yylval.str = res;
		  return STRING;
                }

#.*             { }
;.*             { }

\(|\)		return yytext[0];


.|\s|\n


%%

