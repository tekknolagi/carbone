
void    fixnum_init(void);
VALUE   fixnum_overflow(VALUE operation, VALUE rcv, VALUE other);
VALUE   fixnum_coerce(VALUE operation, VALUE rcv, VALUE other);

extern VALUE  sym_plus,  sym_minus, sym_mul, sym_div,
  sym_lt, sym_gt, sym_cmp,
  sym_uplus, sym_uminus;

