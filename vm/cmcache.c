#include "cmcache.h"

char  cmcache_description[] = m_cmcache_description;

CMC_GLOBALS

/*---------------------------------------*/
#ifdef all_slots_cache
List     *cmcache_selectors = 0;
Selector ...
#endif 





/*---------------------------------------*/
void cmc_init() {

#ifdef hash_methods_cache
  class_method_cache = allocn(cmc_size, cmc_cache_type);
#endif

}


#ifdef all_slots_cache
Selector cmcache_sym2sel(Sym sym) {
  
}
Sym cmcache_sel2sym(Selector sel) {

}
#endif




