#include "fond/method.h"

void     blk_init(void);
Block*   blk_new(List *proc_def, Callable *ca);
Proc*    blk_new_proc(Cell* fp, Block *b);

extern Method *blk_meth_call, *blk_meth_slc;

