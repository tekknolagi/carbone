/* frequence counter

  Copyright (C) 2002 Markus Liedl, markus.lado@gmx.de

  This file is part of Carbone.

  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
*/



// know your frequency...


#include <stdio.h>
#include <assert.h>

#include "fond/rb.h"
#include "fond/value.h"
#include "fond/allocate.h"
#include "fond/string.h"
#include "fond/linked-list.h"
#include "opt/kenneth.h"


unsigned long global_tick = 0;
unsigned long global_tock_period = 1 << global_tock_bits;

List *global_counters = NULL;   // List of Kenneth'es

void ken_init_period(Period *p, long period);

Kenneth *ken_new(long period) {
  Kenneth *k = alloc(Kenneth);
  k->counter = 0;
  ken_init_period(k->p, period);
  return(k);
}

Kenneth *ken_new_n(int num, long *period) {
  Kenneth *k;
  int i;

  assert( num >= 1 );
  k = allocate( (sizeof(Kenneth) + sizeof(Period)*(num-1)) );

  k->num_periods = num;
  k->counter = 0;
  k->last_diff = 0;
  k->counter_last_tock = 0;
  for (i = 0; i < num; i++)
    ken_init_period(k->p+i, period[i]);
  return(k);
}

void ken_init_period(Period *p, long period) {
  p->period = period;
  p->last_value = 0;
  p->average = 0;
}


//--------------------------
// small and fast;
// thread concurrence is no problem, since we may just lose some increments
void ken_count(Kenneth *k) {
  k->counter++;
}

//--------------------------
// everytime a tock arrives, we will update the average values for
// the registered Kenneth'es
//
// avg(new,old[])  =  new + old[0]*f**1 + old[1]*f**2 + old[2]*f**3
//
// somebody knows the name for this kind of median value?
//
// for 
#        define f(old, new)  ((old*7ul)/8ul + new)
//  we get
// f**1 = 0.875,   f**2 = 0.765,   f**3 = 0.669,   f**4 = 0.586,   f**5 = 0.512,
// f**6 = 0.448,   f**7 = 0.392,   f**8 = 0.343, .....            f**20 = 0.069
//
#        define scale_back(x) ((x)/8)
// to get a value comparable to the original diff between new and old[1];
// ( for f(old,new)=(old/2+new),   scale_back(x) would be (x)/2 but for
//   others, i'd need a math book. )
void ken_tock(void) {

  VALUE averager(List *l) {
    int i;
    unsigned long last_diff;
    Kenneth *k = (Kenneth *)(l->head);

    k->last_diff = last_diff = k->counter - k->counter_last_tock;
    k->counter_last_tock = k->counter;

    for(i = 0; i < k->num_periods; i++) {
      Period *p = k->p + i;
      if ( (global_tick>>global_tock_bits) % p->period  == p->period - 1) {
	unsigned long diff = k->counter - p->last_value;
	p->average = f(p->average, diff);
	p->last_value = k->counter;
      }
    }
    return(IT_CONTINUE);
  }

  list_walk(global_counters, averager);
}


void ken_register(Kenneth *k) {
  if (global_counters == NULL)
    global_counters = list_new((VALUE)k);
  else
    list_append(&global_counters, (VALUE)k);
}


#if defined(TEST_KENNETH)
#include <stdlib.h>
#include "math.h"
int main() {
  int num = 5, i, j, cnt, num_tries = 4*1000*1000;
  Kenneth* k[num];
  unsigned long periods[] = {1, 5, 15};

  for(i = 0; i < num; i++) {
    k[i] = ken_new_n(3, periods);
    ken_register(k[i]);
  }

  for (cnt = 0; cnt < num_tries; cnt++) {
    int c = rand() % (num*10);
    
    if (cnt > num_tries/3 && cnt < num_tries/3*2)   c = (num*10)/(c+1);

    c /= 10;

    ken_count( k[c] );    tick();
    ken_count( k[c] );    tick();

    if (cnt % (100*1000) == 0) {
      printf("%8i:", cnt);
      for(i = 0; i < num; i++) {
	printf(" (%i>", i);
	for(j = 0; j < 3; j++) {
	  Period *p = k[i]->p + j;
	  printf(" %6u", (int)(p->average / p->period / 1l));
	}
	printf(")");
      }
      puts("");
    }
  }
  return(0);
}
#endif



