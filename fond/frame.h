void    frm_init(void);

Frame*  frm_create(char *fp);

VALUE   frm_self(Frame *f);
int     frm_offs_self(void);

Frame*  frm_outer(int i, Frame *f);
void    frm_set_outer(int i, Frame *f, Frame *outer);

VALUE*  frm_param(int i, Frame *f);
void    frm_set_param(int i, Frame *f, VALUE v);

VALUE   frm_lvar(int i, Frame *f);
void    frm_set_lvar(int i, Frame *f, VALUE v);

int     frm_offs_param(int i, Callable *ca);
int     frm_offs_outer(int i, Callable *ca);
int     frm_offs_lvar(int i, Callable *ca);

int     frm_idx_param(int i, Callable *ca);
int     frm_idx_lvar(int i, Callable *ca);
