include ../Makefile.conf

CFLAGS = $(CF) -I.. $(GCC_CONF_OPT)

OBJ=symbol.o linked-list.o linked-list-tree.o \
    code.o string.o \
    object.o class.o const.o \
    method.o callable.o block.o \
    fixnum.o \
    allocate.o \
    value.o \
    frame.o

compile: $(OBJ)

tests:	linked-list-test linked-list-tree-test symbol-test

linked-list-test: linked-list.c linked-list.h
	gcc -Wall -lgc -DTEST_LL -I.. symbol.c linked-list.c string.c \
            -o linked-list
	./linked-list

linked-list-tree-test: linked-list-tree.c linked-list-tree.h
	gcc -Wall -lgc -DTEST_LLT -I.. symbol.c linked-list.c \
            linked-list-tree.c string.c ../value.c \
            -o linked-list-tree
	./linked-list-tree

symbol-test:
	gcc -Wall -lgc -DTEST_SYMBOL -I.. symbol.c linked-list.c string.c -o symbol
	./symbol


fixnum.o: fixnum.c fixnum.h ../fond/value.h ../fond/value.c

method.o: method.c method.h ../vm/stack-layout.h