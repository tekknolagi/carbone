void ca_init(void);
void ca_initialize(Callable *ca, Callable *outer,
		   List *params, VALUE rest_array_name,
		   List *lvars, int has_frame,
		   List *tree);


/* location types */
#define VARTYP_PARAM 1
#define VARTYP_LVAR  2

#define NOT_FOUND 0
#define STACK    1
#define FRAME    2

int ca_find_param(Callable *comp_ctx, VALUE param_name, int *ctx, int *idx);
int ca_find_lvar(Callable *comp_ctx, VALUE lvar_name, int *ctx, int *idx);
int ca_find_loc(Callable *comp_ctx, VALUE name,
		int type, int *ctx, int *idx);
List *ca_get_name_list(Callable *compilation_ctx, int type);
int ca_idx_stack_loc(Callable *compilation_ctx, VALUE name, int type);
int ca_svars_base(Callable *ca);
int ca_idx_frame_loc(Callable *compilation_ctx, VALUE name, int type);



