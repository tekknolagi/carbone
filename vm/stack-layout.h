
#if !defined(VM_STACK_LAYOUT_H)
#define VM_STACK_LAYOUT_H

#include "fond/rb.h"
#include "fond/callable.h"

#define fp_pick_at(fp, offs) (*(VALUE*)(((char*)fp)+(offs)))

int stk_callable_offs(void);
int stk_rcv_offs(void);
int stk_num_param_offs(void);
int stk_oldfp_offs(void);
int stk_frame_offs(void);
int stk_param_offs(int i);
int stk_svar_offs(int i);

int stk_param_idx(int i);
int stk_svar_idx(int i);

Callable* stk_callable(char *fp);
VALUE     stk_rcv(char *fp);
int       stk_num_param(char *fp);
char*     stk_oldfp(char *fp);
Frame*    stk_frame(char *fp);
VALUE     stk_param(char *fp, int i);
VALUE     stk_svar(char *fp, int i);

#endif
