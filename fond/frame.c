#include "fond/rb.h"
#include "fond/object.h"
#include "fond/callable.h"
#include "fond/frame.h"
#include "fond/block.h"
#include "comp/cfunc.h"
#include "vm/stack-layout.h"

void frm_init(void) {
  cfunc_declare("frm_create", frm_create, 1, 1);
}

/*
  frm_create()

  create a heap frame with copies of the values of the current stack frame;
  (all other uses of the word frame speak of heap-frames; except
  frame pointer, which _always_ refers to a pointer to the stack)
  local variables are preset with undef

  parameters:
    fp - current frame pointer
*/
  
Frame *frm_create(char *fp) {
  Frame    *frame, *outer_frame;
  char     *outer_fp;
  Callable *ca;
  int      i, is_method;

  ca = stk_callable(fp);
  if (ca == (Callable*) blk_meth_call || ca == (Callable*) blk_meth_slc) {
    ca =  (Callable*) ((Proc*)stk_rcv(fp))->block;
  }

  is_method = (ca->class == class_method);
  assert(is_method || ca->class == class_block);

  frame = allocate(ca->frm_compl_size);
  frame->frame_class = ca;

  if (is_method) {
    outer_fp    = 0;
    outer_frame = 0;
    frame->self  = stk_rcv(fp);
    frame->frame_instance = (Object*) 0; /* [FIXME] BoundMethod */
  }
  else {
    Proc   *frmi;

    frmi = (Proc*) stk_rcv(fp); /* == Proc */
    ca   = (Callable*)frmi->block;

    frame->frame_instance = (Object*) frmi;
    outer_frame           = frmi->outer_frame;
    assert(outer_frame != 0);

    frame->self = outer_frame->self;

    assert(outer_frame->frame_class->class == class_method
	   || outer_frame->frame_class->class == class_block);
    assert(outer_frame->frame_instance == 0 /* [FIXME] BoundMethod */
	   || outer_frame->frame_instance->class == class_proc);
  }

                                // copy the same outers our outer_frame has
  for (i = 0; i < ca->num_outer-1; i++) {
    frm_set_outer(i+1, frame, frm_outer(i, outer_frame));
  }

  if (!is_method) {             // and add outer_frame as the
				// immediate outer
    assert(ca->num_outer > 0);
    frm_set_outer(0, frame, outer_frame);
  }

  for (i = 0; i < ca->params.length; i++) {
    frm_set_param(i, frame, stk_param(fp, i));
  }

  for (i = 0; i < ca->lvars.length; i++) {
    frm_set_lvar(i, frame, rb_undef);
  }

  return(frame);
}

#define in_bytes(x)    ((x) * sizeof(VALUE*))
#define access(p,o)    (*(VALUE*)  (((char*)p)+o)   )





VALUE frm_self(Frame *f) {
  return f->self;
}
int frm_offs_self(void) {
  return offsetof(Frame, self);
}

// param i: which outer
//          (0               == innerst outer;
//           ca->num_outer-1 == outerst outer)
int frm_offs_outer(int i, Callable *ca) {
  int res;
  assert(i >= 0 && i < ca->num_outer);
  res = (offsetof(Frame, dyn)
	 + in_bytes(-i + ca->num_outer -1));
  return(res);
}
int frm_offs_param(int i, Callable *ca) {
  int res;
  assert(i >= 0 && i < ca->params.length);
  res =  (offsetof(Frame, dyn)
	  + in_bytes(ca->num_outer
		     + i));
  return res;
}
int frm_offs_lvar(int i, Callable *ca) {
  int res;
  assert(i >= 0 && i < ca->lvars.length);
  res =  (offsetof(Frame, dyn)
	  + in_bytes(ca->num_outer
		     + ca->params.length
		     + i));
  return res;
}


Frame *frm_outer(int i, Frame *f) {
  return (Frame*) access(f, frm_offs_outer(i, f->frame_class));
}
void frm_set_outer(int i, Frame *f, Frame *outer) {
  access(f, frm_offs_outer(i, f->frame_class)) = (VALUE)outer;
}

VALUE *frm_param(int i, Frame *f) {
  return (VALUE*) access(f, frm_offs_param(i, f->frame_class));
}
void frm_set_param(int i, Frame *f, VALUE v) {
  access(f, frm_offs_param(i, f->frame_class)) = v;
}

VALUE frm_lvar(int i, Frame *f) {
  return access(f, frm_offs_lvar(i, f->frame_class));
}
void frm_set_lvar(int i, Frame *f, VALUE v) {
  access(f, frm_offs_lvar(i, f->frame_class)) = v;
}

int frm_idx_param(int i, Callable *ca) {
  return(frm_offs_param(i,ca)/sizeof(VALUE));
}
int frm_idx_lvar(int i, Callable *ca)  { 
  return(frm_offs_lvar(i,ca)/sizeof(VALUE));
}
