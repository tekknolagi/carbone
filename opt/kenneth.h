
#if !defined(KENNETH_H)
#  define KENNETH_H

#include "fond/allocate.h"


typedef struct Period {
  unsigned long period;
  unsigned long last_value;
  unsigned long average;
} Period;


#define N 1  // or 2 or 3
typedef struct Kenneth {
  unsigned long  counter;
  unsigned long  counter_last_tock;
  unsigned long  last_diff;
  short          num_periods;
  Period         p[N];
} Kenneth;


#define global_tock_bits 16

#define tick() { \
  if (   global_tick  >> global_tock_bits  != \
      (++global_tick) >> global_tock_bits)     ken_tock(); }

// number of global ticks
extern unsigned long global_tick;

// all periods are multiples of global_tock
extern unsigned long global_tock_period;


Kenneth *ken_new(long periods);
Kenneth *ken_new_n(int num, long *periods);
void ken_tock(void);
void ken_count(Kenneth *k);


#endif
