LDFLAGS:=-lgc

E_CFLAGS:=-O3 -g


tree-eb:
	rm -f boehm-tree
	make boehm-tree \
             CFLAGS="$(CFLAGS) -DEXTRA_BYTES=$(EB) -DFREE_LEVEL=$(FL) $(E_CFLAGS)"
	mv boehm-tree tree-eb-$(EB)


rep:
	echo ----------------------------------- $(EB) -----------
	make tree-eb
	($(EXTRA)  ./tree-eb-$(EB) || true)

a:
	make -s rep EB=0
	make -s rep EB=4
	make -s rep EB=8
	make -s rep EB=16
	make -s rep EB=32
	make -s rep EB=64
	make -s rep EB=128


d:
	make -s a >rep_nofree
	make -s a CFLAGS=-DCUT >rep_cutting
	diff -u rep_nofree rep_cutting || true

#i:
#	((( vmstat 1 | ruby -ne 'puts "\n"+$$_' &)  && \
#	   sleep 1 && make -s a CFLAGS=-DCUT); exit)


