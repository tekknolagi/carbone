#if !defined(RB_H)
#define RB_H

#include "fond/symbol.h"
#include "fond/linked-list.h"
#include "configuration.h"

struct Object;
struct Code;
struct Method;
struct Block;
struct Proc;

#include "fond/object_header.h"

// N == arbitrary many
#define N 0


/* ----------------------------------------------------- CountedList ----- */
typedef struct CountedList {
  List           *names;           // List of symbols or 0 if empty
  long           length;
} CountedList;


/* ----------------------------------------------------- Callable ----- */
// Callable is an abstract superclass of Method and Block.

#define CALLABLE_FIELDS\
  /*struct Class  *class;                    OBJECT_HEADER */\
 OBJECT_HEADER \
  \
  /*
    outer is a ptr to the Block or Method. It is the lexically
    enclosing Callable. When a Block is immediatly inside a Class body
    it's outer will point to the corresponding anonymous method.  A
    Method has outer == 0, because the local variables of its
    enclosing context are not accessible. */\
  struct Callable *outer;                  \
  int            has_frame;                /* 0 or 1 */ \
  CountedList    params;                   \
  CountedList    lvars;                    \
  VALUE          rest_param_array;         /* name of rest-variable or rb_nil*/\
                                           \
  List           *tree;                    /* for recompilation */\
  struct Code    *code;                    /* for execution */\
  void           *cache_code;              /* from Code (1) */\
  void           **cache_insn;             /* from Code (2) */\
                                           \
  int            frm_compl_size;           /* in bytes */\
  int            num_outer;                /* to calculate field offsets */


typedef struct Callable {
  CALLABLE_FIELDS
} Callable;

#if defined(NDEBUG)
# define DEBUG_MODE(x)
#else
# define DEBUG_MODE(x) x
#endif


/* ----------------------------------------------------- Frame ----- */
typedef struct Frame {
  Callable        *frame_class;         /* Method or Block */
  struct Object   *frame_instance;      /* 0 (BoundMethod) or Proc */
  VALUE           self;

  void* dyn[0];                         /* begin of dynamic end */
  struct Frame    *outer[N];
  VALUE           param[N];
  VALUE           lvar[N];

  // [FIXME] some information where super should continue to search
} Frame;


/*              Now the real Objects:             */


/* ----------------------------------------------------- Object ----- */
typedef struct Object {
  OBJECT_HEADER
  VALUE           ivar[N];    // instance variables;
} Object;



/* ----------------------------------------------------- Class ----- */
typedef struct  Class {
  OBJECT_HEADER
  VALUE         name;
  struct Class  *super;

  List          *mixins;        // modules are also classes; list of Class*
  long          *mixin_offs;    // ivars of mixins have a class-specifique offset

  List          *method_names;
  struct Method **methods;

  List          *class_defs;    // list of anonymous methods; class def code

  CountedList   lvars;          // class def local variables
  CountedList   ivars;          // names of instance variables
  CountedList   cvars;          // names of class variables
  VALUE         *cvar_values;   // array of values for the class variables

  int           all_ivars;      // number of ivars (mixins and super included)
  int           inst_compl_size;// size of instances in Bytes
			        // (header included) 

  List          *body;          // the class definition

  //  CLASS_STRUCT_EXTRA_FIELDS   // from vm/cmcache.h
} Class;




/* ----------------------------------------------------- Method ----- */
typedef struct Method {
  CALLABLE_FIELDS

  struct Class   *recv_class;      // the class this method belongs to
  VALUE          name;

  VALUE          proc;             // name of proc-slot parameter or rb_nil

  List           *optionals;       // optional parameters

  /*
     int         scope;            // public/private/protected
  */
} Method;



/* ----------------------------------------------------- Block ----- */
typedef struct  Block {
  CALLABLE_FIELDS
  /*
    A Block instance describes its Proc instances.
    For each literal `{ ... }' and `do ... end' in the source code
    exists exactly one Block instance in the running program.     */

  VALUE          dont_wrap_single;   // boolean

} Block;



/* ----------------------------------------------------- Proc ----- */
typedef struct Proc {
  OBJECT_HEADER
  /*
    For each Block instance exist that many instances the program
    wishes to create.
  */
  Block          *block;
  Frame          *frame;        // ptr to Frame or 0
  Frame          *outer_frame;  // ptr to outer Frame or 0; importent
				// for Procs without an own Frame
} Proc;


/* ----------------------------------------------------- Code ----- */
typedef struct  Code {
   OBJECT_HEADER

  // A Code object represents either:
  void         *code;             // (1) a sequence of VM instructions ...
  void         **insn;            // (2) one VM instruction

  // to form a legal Code object one of those two has to be zero.

  // concerning (1)
  void         *code_end;    // disassemble
  void         *engine;      // which engine can execute this code? (dbg, nodbg)
  void         *prim;        // which set of primitives was used to gen this code

  //List *assumptions;       // for invalidation of code
} Code;

/* ----------------------------------------------------- Array ----- */
typedef struct Array {
  OBJECT_HEADER
  VALUE        *values;
  long         length;
  long         capacity;
} Array;


/* ----------------------------------------------------- String ----- */
typedef struct String {
  OBJECT_HEADER
  long          length;
  char          *string;
} String;





#endif




