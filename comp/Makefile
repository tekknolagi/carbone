include ../Makefile.conf
.PHONY: obj
CFLAGS+=-I.. $(GCC_CONF_OPT)

OBJ=generator.o cfunc.o

compile: $(OBJ)

generator.o: generator.c ../configuration.h ../vm/stack-layout.h



