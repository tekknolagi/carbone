include ../Makefile.conf


CFLAGS=$(GCC_CONF_OPT) -I..
# -DTHREADING_SCHEME=10


VMGEN_TARGETS=gen/rb-vm.i gen/rb-disasm.i gen/rb-gen.i \
        gen/rb-labels.i gen/rb-profile.i gen/rb-peephole.i

OBJS=datatypes.o engine.o peephole.o profile.o disasm.o \
     cmcache.o builtins.o desc.o stack-layout.o


ifeq "$(ENGINE_DEBUG)" "true"
  OBJS += engine-debug.o
endif

DEBUG_CC_OPTS     =  -Wall -g -O0 -I..
DEBUG_ENGINE_OPTS = -DVM_DEBUG -DVM_PROFILING -Dengine=engine_debug


.PHONY: build
build: $(OBJS)

$(VMGEN_TARGETS): rb.vmg
	cd gen && vmgen ../rb.vmg
	touch $(VMGEN_TARGETS)   ## ??? why does vmgen not touch ???

                                  #make_ascending_labels
gen/rb-vm-lbl.i: gen/rb-vm.i
	ruby -e 'id=0; \
                 File.new("gen/rb-vm.i").each_line {|l| \
                   if l["retry_send"] then \
                     if l["<new-id>"] then id += 1 end; \
                     l.gsub!(/<(new-)?id>/, id.to_s) \
                   end; \
                   puts l \
                 }' >  gen/rb-vm-lbl.i

rb.vmg: rb-inst.vmg ../builtins/all_builtins.vmg rb-super.vmg
	cat $^ >$@


rb-super.vmg: $(prof)
ifndef prof
	echo >$@
else
	./si-choice $(prof) >$@
endif


# carbone specials:
gen/rb-desc.i: gen/rb-labels.i
	cd misc && ruby insns2str.rb

engine_NEXT_P2.h: engine.c
	cd misc && ruby extract_NEXT_P2.rb >../$@


#

desc.o: desc.c gen/rb-desc.i

datatypes.o: datatypes.c vm.h

peephole.o: peephole.c gen/rb-peephole.i vm.h

profile.o: profile.c gen/rb-profile.i vm.h

disasm.o: disasm.c gen/rb-disasm.i vm.h


engine.o: CFLAGS := $(CFLAGS) $(VM_ENGINE_O_EXTRA_OPT)
engine.o: engine.c gen/rb-vm-lbl.i gen/rb-labels.i vm.h  \
                              engine_NEXT_P2.h stack-layout.h

engine-debug.o : CFLAGS = $(DEBUG_CC_OPTS) $(DEBUG_ENGINE_OPTS)
engine-debug.o : engine.c gen/rb-vm-lbl.i gen/rb-labels.i vm.h \
                              engine_NEXT_P2.h  stack-layout.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o gen/* rb.vmg

E: engine.c gen/rb-vm.i gen/rb-labels.i vm.h
	gcc -E -o engine.E engine.c -I..
	cat engine.E | grep -v '^\s*$$' | less

engine.gdis: engine.o
	gdb --batch -x disas_insns.gdb ../gen > $@

