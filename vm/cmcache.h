#if !defined(CMCACHE_H)
#define CMCACHE_H

#include "configuration.h"
#include "fond/rb.h"
#include "fond/value.h"
#include "fond/allocate.h"

extern char  cmcache_description[];
void cmc_init(void);


/* [[FIXME]]  cache negative information;
   the case when a method with a given selector does not exist
   and :missing should be sent
*/



/*-----------------------------------------------------------------*/
#if conf_cmcache==0
#  define no_method_cache

#  define m_cmcache_description \
          "none"
// lookup method each time a message gets sent

#  define CLASS_STRUCT_EXTRA_FIELDS


#  define CMC_GLOBALS
#  define CMC_GLOBALS_EXTERN
          // find it again in fond/method.h

#  define Selector Sym
#  define sym2sel(sym) (sym)
#  define sel2sym(sel) (sel)

#  define lookup_method(class, sel, method) \
     method = class_find_method(class, sel)

#endif
/*-----------------------------------------------------------------*/





/*-----------------------------------------------------------------*/
#if conf_cmcache==1
#  define all_slots_cache
/* fat cache;
   keep a complete array of effective methods for each class;
   not only for selectors that may really be used for this class, but
   for the global list of possible selectors. (that are: all selectors)
   this way we can get the method from the selector by a simple memory
   lookup;
   sel is not `Sym' (index of symbol) but `Selector` (== index in
   slot-arrays)
   if a new symbol gets used as a selector (e.g. a method with a
   till that moment unused name gets defined) all classes have to be
   updated (e.g. grow all caches by one slot)

   pro   : dispatch-expense is very much c++ virtuals like
   contra: lots of mem used; unusable for big 'scripts'
*/
#  define m_cmcache_description \
          "all slots"

#  define CLASS_STRUCT_EXTRA_FIELDS   struct Method* eff_methods;

#  define expose_method(class, sym, method) \
    sym_is_a_selector(sym); \
    cmcache_add_slot

#  define Selector long
#  sym2sel(sym) cmcache_sym2sel(sym)
#  sel2sym(sel) cmcache_sel2sym(sel)

#  define lookup_method(class, sel, method) \
   ({  method = class->eff_methods[sel]; \
       if (method == 0) \
          class->eff_methods[sel] =  \
                    method = class_find_method(class, sel); })

// fat-cache local:
#  define sym_is_a_selector(sym) \
     walk through all classes and add one slot


#endif
/*-----------------------------------------------------------------*/





/*-----------------------------------------------------------------*/
#if conf_cmcache==2
#  define top_slots_cache
/* top slots;
   similiar to fat cache, except, that only the globally most often
   used slots are kept.
   This should provide fat-cache-like performance for big
   applications. 
   
   At runtime we inspect which selectors get used most frequently.
   In regular intervalls we build a size-limited array of the
   corresponding effective methods for each class and a global array
   that maps top selectors to an index for the small array.
   Then the lookup works in two memory lookups (ignoring the
   fault-case):
   class.top_eff_methods[selector2small_array_idx[Selector]]
*/
#  define m_cmcache_description \
          "top slots"


#endif
/*-----------------------------------------------------------------*/



/*-----------------------------------------------------------------*/
#if conf_cmcache==3
#  define hash_methods_cache
/* one global [class,sel] hash for all classes and all selectors
*/
#  define m_cmcache_description \
          "hash"

typedef struct s_cmc_method_cache_entry {
  void           *meth;        // Method*
  long            name_sym;    // Sym
  void           *eff_class;   // Class*
} cmc_method_cache_entry;

#define cmc_cache_type cmc_method_cache_entry

//#  define Selector Sym
//#  define sym2sel(sym) (sym)
//#  define sel2sym(sel) (sel)

#  define lookup_method(class, sel, method) \
    {                                       \
       cmc_method_cache_entry *m = class_method_cache + cmc_hash(class,sel);  \
       method = (Method*)m->meth;              \
       if (method == 0 ||                      \
           (Class*)m->eff_class != class ||    \
           (Sym)m->name_sym != sel)            \
       {                                       \
          if (method != 0)                     \
            fprintf(stderr, "vm/cmcache.h: collision in class-method cache.\n");\
          m->meth = method = class_find_method(class, sel);         \
          m->name_sym = sel;       \
          m->eff_class = class;    \
    }  }

          //printf("[cmc] for class: %p :: found method %p\n", class, method); 
          //printf("[cmc] lookup method: %s\n", sym_sym2str(sel));   

// locals
#define cmc_shift 16
#define cmc_size (1<<cmc_shift)
#define cmc_mask (cmc_size-1)
#define cmc_hash(class,sel)  ((((VALUE)class)     ^  \
                               ((VALUE)sel))      & cmc_mask)

/*
#define cmc_hash(class,sel)  ((((VALUE)class)             ^  \
                              (((VALUE)class)>>cmc_shift) ^  \
                               ((VALUE)sel))              & cmc_mask)
*/

#  define CLASS_STRUCT_EXTRA_FIELDS
#  define CMC_GLOBALS cmc_cache_type* class_method_cache;
#  define CMC_GLOBALS_EXTERN  extern CMC_GLOBALS


#endif
/*-----------------------------------------------------------------*/



#endif
