include ../../Makefile.conf

CFLAGS=$(GCC_CONF_OPT) -I../..
LDFLAGS=-lgc


OBJ=lgram.o lex.yy.o y.tab.o

obj: $(OBJ)


tests: test
	(for n in *.lgram ; do echo "-----------------------------------"; \
	echo $$n; echo $$n|awk 'gsub(/./,"-")'; \
	./test $$n ; done)

test: y.tab.o lex.yy.o \
           ../../fond/linked-list.o \
           ../../fond/linked-list-tree.o \
           ../../fond/symbol.o \
           ../../fond/string.o \
           lgram.o \
           test.o 

lex.yy.c: p.l lgram.h
	flex p.l

y.tab.c y.tab.h: p.y lgram.h
	yacc -v -d p.y


lgram.h: ../../fond/linked-list.h ../../fond/symbol.h ../../fond/value.h
