

#include <stdio.h>
#include "fond/value.h"

int main() {
  long i;
  VALUE tmp;

  for (i = -50; i < 100; i += 13) {

    tmp=long2value(i);
    printf("trans from %8x to %8x to %8x\n",
	   i,
	   tmp,
	   value2long(tmp));
	   

  }

}
