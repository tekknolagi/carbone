gen: rb
	mv rb gen
	size gen


Makefile.conf configuration.h: configuration.rb configure.rb
	ruby configure.rb
	make clean

include Makefile.conf

LGRAM_O=comm/lgram/lgram.o comm/lgram/y.tab.o comm/lgram/lex.yy.o

COMP_O=comp/generator.o comp/cfunc.o

FOND_O=fond/linked-list.o fond/linked-list-tree.o fond/symbol.o fond/code.o \
       fond/string.o fond/fixnum.o fond/value.o \
       fond/object.o fond/class.o fond/const.o \
       fond/allocate.o fond/frame.o \
       fond/callable.o fond/method.o fond/block.o

VM_O=vm/datatypes.o   vm/engine.o       vm/peephole.o   vm/profile.o \
     vm/disasm.o      vm/cmcache.o      \
     vm/builtins.o    vm/desc.o         vm/stack-layout.o
ifeq "$(ENGINE_DEBUG)" "true"
  VM_O += vm/engine-debug.o
endif

BASE=main.o

CFLAGS=$(GCC_CONF_OPT) -I$(shell pwd)
LDFLAGS=$(LDFLAGS_GC) $(LDFLAGS_OTHER)

subdirs: $(BASE)
	make -C comm
	make -C builtins
	make -C vm
	make -C fond
	make -C comp

main.o: main.c main.h

rb: subdirs
	gcc -o rb $(LDFLAGS) $(FOND_O) $(LGRAM_O) $(COMP_O) $(VM_O) $(BASE)




tests: machine ?=  ./gen
tests: wc      ?=  tests/t*_*.lg
tests: do-tests

all-tests: machine ?=  ./gen
all-tests: wc       =  tests/*.lg
all-tests: do-tests

do-tests: $(machine)
	\
for i in $(wc); do\
  echo; echo ----------------------- $$i:;\
  $(machine) $(flags) $$i || echo ' -- PROBLEM(s) -- ';\
done;\
echo



clean:
	rm -f core $(FOND_O) $(LGRAM_O) $(COMP_O) $(VM_O) $(BASE)
	rm -f builtins/*.vmg builtins/*_bi_decl.i

arch_clean: clean
	echo >vm/rb-super.vmg
	rm Makefile.conf
	rm -f gen_base gen vm/rb.vmg
	(find . -name "*.log" -or \
                -name "*.prof" -or \
                -name "*.vm" -or \
                -name "*.E" -or \
                -name "*.s") | xargs rm -f
	rm -f vm/gen/*.i
	cd comm/lgram && rm -f y.tab.c lex.yy.c *.o y.output


arch: f=Carbone-archives/carbone_$(shell date '+%Y%m%d_%H%M').tar.bz2
arch:
	make arch_clean
	cd .. && tar cjf $(f) carbone


mes: rb
	make -C opt/measurement compile
	gcc -o mes opt/measurement/cfunc.o \
	           $(LDFLAGS) $(FOND_O) $(LGRAM_O) $(COMP_O) $(VM_O) $(BASE)

%.bench: % $(machine)
	echo "[MEASURE] machine $(machine)" >>$(into)
	$(machine) -V $(bench_param) 2>>$(into)
	(size gen  && echo) >>$(into)
	time -a -o $(into) $(machine) $(bench_param) $<
	echo >>$(into)

%.lg.prof: %.lg ./gen
	./gen -Sp $< 2>$@

# make optimized interpreter
# use like  `make opt t=code_for_training.lg'
opt: $(t)
	echo "-------------------- measure base interpreter:" > $(t).log
	make $(t).bench   machine=./gen  bench_param=-VS  into=$(t).log
	make $(t).prof
	make -C vm rb-super.vmg prof=../$(t).prof
	echo "------------------------------ opt interpreter:" >>$(t).log
	make gen
	cp gen $(t).vm
	make $(t).bench   machine=$(t).vm  bench_param=-V  into=$(t).log
	echo >>$(t).log

o: opt
	cat $(t).log


.PHONY: o opt test tests clean arch_clean subdirs small


complexity: t=*
complexity:
	@echo
	@find . -name "$(t)" -type f \
          | grep -vE '(/\.#)|CVS|((\.o|~)$)' \
          | xargs --verbose wc -l \
          | sort -rg

all-complex:
	@make -s complexity t='*.c'
	@make -s complexity t='*.h'
	@make -s complexity t='*.rb'
	@make -s complexity t='*.lg'
	@make -s complexity t='*.decl'
	@make -s complexity t='*.vmg'

