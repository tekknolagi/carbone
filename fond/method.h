#include "vm/vm.h"
void meth_init(void);

Method* meth_from_list(Class *c, List *m);
void    meth_from_builtin(Class *class, char *sel_str, char *insn_name);

Code*   meth_generate(Method *m, int debug);


void meth_report_dyn_recomp_unsupp(VALUE rcv, VALUE sel);
void meth_report_does_not_exist(VALUE rcv, VALUE sel);

void meth_report_wrong_num_parameters(VALUE rcv, char *meth, VALUE num_par);
void meth_report_wrong_num_parameters2(Inst *ip, Cell *sp, char *fp);


#include "vm/cmcache.h"
CMC_GLOBALS_EXTERN
