#include "vm/vm.h"
#include "vm/stack-layout.h"
/*
  The definitions here are used for frame-pointer relative access to the
  values on the stack 

  The offsets FP_RCV to FP_OLDFP depend directly on the parameter list
  of the `send' instruction in rb-inst.vmg.
*/

#define FP_PARAM(x)  ((5+(x))*sizeof(Cell))
#define FP_RCV       (4*sizeof(Cell))
#define FP_SELF      FP_RCV
#define FP_CALLABLE  (3*sizeof(Cell))
#define FP_INPAR     (2*sizeof(Cell))
#define FP_RETIP     (1*sizeof(Cell))
#define FP_OLDFP     (0)
#define FP_SVAR(x)   ((-1-(x))*sizeof(Cell))

// If this Callable has_frame, then the pointer to the Frame is the
// first (and only) svar; 
#define FP_FRAME     FP_SVAR(0)


#include "fond/rb.h"


int stk_param_offs(int i)    { return FP_PARAM(i); }
int stk_rcv_offs(void)       { return FP_RCV;      }
int stk_callable_offs(void)  { return FP_CALLABLE; }
int stk_num_param_offs(void) { return FP_INPAR;      }
int stk_oldfp_offs(void)     { return FP_OLDFP;    }
int stk_frame_offs(void)     { return FP_FRAME;    }
int stk_svar_offs(int i)     { return FP_SVAR(i);  }

int stk_param_idx(int i)     { return (FP_PARAM(i)/sizeof(Cell)); }
int stk_svar_idx(int i)      { return (FP_SVAR(i)/sizeof(Cell));  }

Callable* stk_callable(char *fp) {
  return (Callable*) fp_pick_at(fp, FP_CALLABLE);
}

VALUE stk_rcv(char *fp) {
  return fp_pick_at(fp, FP_RCV);
}

int stk_num_param(char *fp) {
  return (int) fp_pick_at(fp, FP_INPAR);
}

char* stk_oldfp(char *fp) {
  return (char*) fp_pick_at(fp, FP_OLDFP);
}

Frame* stk_frame(char *fp)  {
  return (Frame*) fp_pick_at(fp, FP_FRAME);
}

VALUE stk_param(char *fp, int i) {
  return fp_pick_at(fp, FP_PARAM(i));
}

VALUE stk_svar(char *fp, int i) {
  return fp_pick_at(fp, FP_SVAR(i));
}

