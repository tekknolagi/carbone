

#include <stdio.h>
#include <assert.h>

#include "fond/value.h"
#include "value.c"

int main() {

  long   i;
  VALUE  val;
  long   re;

  for (i = 1000; i > -1000; i -= 117) {
    val = long2value(i);
    re = value2long(val);

    assert(is_immediate(val));
    assert(!is_indirect(val));
    assert(is_fixnum(val));

    printf(" %5li => (VALUE 0x%08x) => %5li \n", i, val, re);
  }

  return(0);
}
