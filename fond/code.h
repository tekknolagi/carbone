
#if !defined(CODE_H)
#define CODE_H

#include "fond/rb.h"
#include "fond/value.h"
#include "fond/allocate.h"
#include "vm/vm.h"
#include "fond/object.h"

void    code_init(void);
Code    *code_new_and_block(int use_debug_engine);
void    code_release_mempool(Inst *till);
void    code_dump_stdout(Code *c);
VALUE   code_invoke(Code *code, int trace, int profile);
void    code_dump_profile(void);

// set like you want...
#define CODE_LENGTH_MAX 200000
// vm/peephole.c needs to know the begin and the end of code; so we have
// just one memory block of code. This will get a problem for dynamic
// recompilation later.


#endif
