
#include <gc/gc.h>
#define take GC_MALLOC

// #include <malloc.h>
// #define take malloc

#include <assert.h>

#include <stdio.h>
#include <malloc.h>

#include "fond/value.h"

int main(int argc, char **argv) {

  int i, j;

  for( j = 0; j < 100000; j++)
    for (i = 0; i < 256; i += 13)
      assert( is_indirect(take(i)) );

  return(0);
}
