#include "stdio.h"
#include "fond/value.h"
#include "fond/rb.h"
#include "fond/linked-list.h"
#include "fond/linked-list-tree.h"
#include "fond/object.h"
#include "fond/callable.h"
#include "fond/frame.h"
#include "vm/stack-layout.h"

List *ca_lex_border;
void ca_find_block_callback(List*, Object *user_param);

void ca_init(void) {
  ca_lex_border = list_new2(sym_lit, sym_quote);

}

void ca_initialize(Callable *ca, Callable *outer,
		   List *params, VALUE rest_array_name,
		   List *lvars, int has_frame,
		   List *tree)
{
  assert(ca->class != class_method || outer == 0);
  ca->outer = outer;

  assert(params == 0 || params->class == class_list);
  assert(lvars == 0 || lvars->class == class_list);

  ca->params.names = params;
  ca->params.length = list_length(params);

  ca->lvars.names = lvars;
  ca->lvars.length = list_length(lvars);

  ca->rest_param_array = rest_array_name;
  assert(is_symbol(rest_array_name) || rest_array_name == rb_nil);

  if (outer == 0)
    ca->num_outer = 0;
  else 
    ca->num_outer = outer->num_outer + 1;


  ca->frm_compl_size = (sizeof(Frame) 
			+ sizeof(VALUE) * (ca->num_outer
					   + ca->params.length
					   + ca->lvars.length));

  ca->tree = tree;
  ca->code = ca->cache_code = ca->cache_insn =    0;

  ca->has_frame = has_frame;
}



/*
  gen_find_param() and gen_find_lvar():

  IN:  a Callable and the name of a parameter or lvar
  OUT: Information on how to navigate to it
         ctx == 0     the own callable that contains this var
         ctx >  0     an outer callable that contains this var

         idx:         frame or stack relative index; (can be <0 for stack) 

  return: 0      not found
          STACK  variable is laying on stack
          FRAME  variable is in the (int)ctx outer frame
                             in position (int)offs
*/

int debug_ca_find_param = 0;
int debug_ca_find_loc = 0;

int ca_find_param(Callable *comp_ctx, VALUE param_name, int *ctx, int *idx)
{
  int ret;
  ret = ca_find_loc(comp_ctx, param_name,
		    VARTYP_PARAM, ctx, idx);

  DEBUG_MODE({
    List *t = comp_ctx->params.names;
    if (debug_ca_find_param) {
      printf("ca_find_param(<ca params=<");
      while (t){
	printf("%s ", sym_sym2str(car(t)));
	t = cdr(t);
      }
      printf(">>,%s) ==> ctx = %i, idx = %i\n",
	     sym_sym2str(param_name), *ctx, *idx);
    }
  })

  return ret;
}

int ca_find_lvar(Callable *comp_ctx, VALUE lvar_name, int *ctx, int *idx)
{
  int ret;
  ret = ca_find_loc(comp_ctx, lvar_name,
		    VARTYP_LVAR, ctx, idx);
  return ret;
}

int ca_find_loc(Callable *comp_ctx, VALUE name,
		 int type, int *ctx, int *idx)
{
  List      *loc_list;
  Callable  *outer;
  int       frm, ret;

  if (debug_ca_find_loc)
    printf("ca_find_loc(ca, %s, %i) ", sym_sym2str(name), type);

  #define RET(x) {ret = x; goto ret;}

  /* does the Callable comp_ctx itself contain this name? */
  loc_list = ca_get_name_list(comp_ctx, type);
  if (list_contains(loc_list, name)) {
    if (!comp_ctx->has_frame) {
      *ctx = -1;
      *idx = ca_idx_stack_loc(comp_ctx, name, type);
      RET(STACK);
    } else {
      *ctx = 0;
      *idx = ca_idx_frame_loc(comp_ctx, name, type);
      RET(FRAME);
    }
  }


  /* does an outer Callable contain this name? */
  outer = comp_ctx;
  frm = 0;
  while (1) {
    outer = outer->outer;
    frm++;

    if (outer == 0) RET(NOT_FOUND);

    assert(outer->has_frame);

    loc_list = ca_get_name_list(outer, type);
    if (list_contains(loc_list, name)) {
      *ctx = frm;
      *idx = ca_idx_frame_loc(outer, name, type);
      RET(FRAME);
    }

  }
 ret:
  if (debug_ca_find_loc)
    printf(" => ctx=%i, idx=%i, ret=%i\n", *ctx, *idx, ret);

  return(ret);
}


List *ca_get_name_list(Callable *comp_ctx, int type) {
  if (type == VARTYP_PARAM)     return comp_ctx->params.names;
  else if (type == VARTYP_LVAR) return comp_ctx->lvars.names;
  else assert(0);
}

/*
  Get the index of a param or lvar laying on the stack;
  The stack layouts of different Callables are respected
*/
int ca_idx_stack_loc(Callable *compilation_ctx, VALUE name, int type) {
  int  varpos, pos;
  List *name_list;

  name_list = ca_get_name_list(compilation_ctx, type);
  pos = list_find(name_list, name);
  assert(pos >= 0);

  if (type == VARTYP_PARAM) {
    varpos = compilation_ctx->params.length - 1 - pos;
    return(stk_param_idx(varpos));
  }
  else if (type == VARTYP_LVAR) {
    varpos = pos;
    return(ca_svars_base(compilation_ctx)
	   +  stk_svar_idx(varpos));
  }
  assert("not reached"&&0);
}
/*
  each callable can have its specific stack layout for svars

  currently there are four cases:               stack contains:

  method with stack frame                                   lvars
  method with Frame (i.e. heap frame)         Frame*
  block with stack frame                      outer_frame*, lvars
  block with Frame (i.e. heap frame)          Frame*

  (imagineable are:
      *  cache self on stack
      *  cache outer_frame* on stack) for the two heap Frame cases
*/
int ca_svars_base(Callable *ca)
{
  if (ca->outer || ca->has_frame)
    return(1);
  return(0);
}


/*
  Get the index of a param or lvar in a Frame;
  The stack layouts of different Callables are respected
*/
int ca_idx_frame_loc(Callable *comp_ctx, VALUE name, int type) {
  int  varpos;
  List *name_list;

  name_list = ca_get_name_list(comp_ctx, type);
  varpos = list_find(name_list, name);
  assert(varpos >= 0);

  if (type == VARTYP_PARAM)
    return(frm_idx_param(varpos, comp_ctx));
  else if (type == VARTYP_LVAR)
    return(frm_idx_lvar(varpos, comp_ctx));
  assert("not reached"&&0);
}

