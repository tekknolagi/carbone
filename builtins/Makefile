
all_builtins.vmg: fixnum_bi.vmg
	cat *_bi.vmg > all_builtins.vmg


%_bi_decl.i %_bi.vmg: %.decl
	ruby trans.rb $* > $*_bi.vmg

