#include "vm/vm.h"

void      gen_init(void);
Code*     gen_code(List *tree, int debug);
Code*     gen_code_callable(Callable *ca);

void      gen_check_params(Inst **c, Callable *ca);
void      gen_alloc_lvars(Inst **c, Callable *ca);
void      gen_drop_lvars(Inst **c, Callable *ca);
void      gen_code_end(Inst **c);

void      gen_add_compile_deferred(Object *blk);
void      gen_compile_deferred(void);




