#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

#ifndef YYSTYPE
typedef union {
  struct List *list;
  VALUE       value;
  char        *str;
} yystype;
# define YYSTYPE yystype
#endif
# define	NUM	257
# define	IDENT	258
# define	STRING	259


extern YYSTYPE yylval;

#endif /* not BISON_Y_TAB_H */
