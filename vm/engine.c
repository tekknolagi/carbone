/* vm interpreter wrapper

  Copyright (C) 2002 Markus Liedl, markus.lado@gmx.de
  Copyright (C) 2001 Free Software Foundation, Inc.

  This file is part of Carbone.
  This file was part of Gforth.

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
// #define INLINE_CONVERTERS inline static

#define MAY_BE_INLINED inline static

#include <stdlib.h>
#include <string.h>
#include "fond/rb.h"
#include "fond/value.h"
#include "fond/value.c"
#include "configuration.h"
#include "fond/class.h"
#include "fond/symbol.h"
#include "fond/object.h"
#include "fond/fixnum.h"
#include "fond/method.h"    // meth_report_wrong_num_parameters is hidden there
#include "comp/generator.h"
#include "vm/vm.h"
#include "vm/stack-layout.h"
#include "vm/builtins.h"



/* type change macros; these are specific to the types you use, so you
   have to change this part */
#define vm_Cell2i(x) ((Cell)(x))
#define vm_i2Cell(x) (x)

#define vm_Cell2target(x) ((Inst *)(x))
#define vm_target2Cell(x) ((Cell)(x))

#define vm_Cell2cfunc(x) ((void *)(x))
#define vm_cfunc2Cell(x) ((Cell)(x))

#define vm_Cell2Cell(x) ((Cell)(x))

#define vm_Cell2self(x) ((VALUE)(x))
#define vm_self2Cell(x) ((Cell)(x))

#define vm_Cell2sel(x) ((Sym)(x))
#define vm_sel2Cell(x) ((Cell)(x))

#define vm_Cell2rcv(x) ((VALUE)(x))
#define vm_rcv2Cell(x) ((Cell)(x))

#define vm_Cell2proc(x) ((VALUE)(x))
#define vm_proc2Cell(x) ((Cell)(x))

#define vm_Cell2oldfp(x) ((void*)(x))
#define vm_oldfp2Cell(x) ((Cell)(x))

#define vm_Cell2retip(x) ((Inst*)(x))
#define vm_retip2Cell(x) ((Cell)(x))

#define vm_Cell2ptr(x) ((Cell*)(x))
#define vm_ptr2Cell(x) ((Cell)(x))

#define vm_Cell2other(x) ((VALUE)(x))
#define vm_other2Cell(x) ((Cell)(x))

#define vm_Cell2meth(x) ((Method*)(x))
#define vm_meth2Cell(x) ((Cell)(x))

#define vm_Cell2class(x) ((Class*)(x))
#define vm_class2Cell(x) ((Cell)(x))


#ifdef USE_spTOS
#  define IF_spTOS(x) x
#else
#  define IF_spTOS(x)
#endif

#ifdef VM_DEBUG
#define NAME(_x) {trace_insn(_x,ip,fp,sp);}
#else
#define NAME(_x)
#endif

/* different threading schemes for different architectures; the sparse
   numbering is there for historical reasons */

/* here you select the threading scheme; I have only set this up for
   386 and generic, because I don't know what preprocessor macros to
   test for (Gforth uses config.guess instead).
   Anyway, it's probably best to build them all and select the fastest
   instead of hardwiring a specific scheme for an architecture. */
#ifndef THREADING_SCHEME
#ifdef i386
#define THREADING_SCHEME 8
#else
#define THREADING_SCHEME 5
#endif
#endif /* defined(THREADING_SCHEME) */

#if THREADING_SCHEME==1
/* direct threading scheme 1: autoinc, long latency (HPPA, Sharc) */
#  define NEXT_P0	({cfa=*ip++;})
#  define IP		(ip-1)
#  define SET_IP(p)	({ip=(p); NEXT_P0;})
#  define NEXT_INST	(cfa)
#  define INC_IP(const_inc)	({cfa=IP[const_inc]; ip+=(const_inc);})
#  define DEF_CA
#  define NEXT_P1
#  define NEXT_P2	({goto *cfa;})
#endif

#if THREADING_SCHEME==3
/* direct threading scheme 3: autoinc, low latency (68K) */
#  define NEXT_P0
#  define IP		(ip)
#  define SET_IP(p)	({ip=(p); NEXT_P0;})
#  define NEXT_INST	(*ip)
#  define INC_IP(const_inc)	({ip+=(const_inc);})
#  define DEF_CA
#  define NEXT_P1	({cfa=*ip++;})
#  define NEXT_P2	({goto *cfa;})
#endif

#if THREADING_SCHEME==5
/* direct threading scheme 5: early fetching (Alpha, MIPS) */
#  define CFA_NEXT
#  define NEXT_P0	({cfa=*ip;})
#  define IP		((Cell *)ip)
#  define SET_IP(p)	({ip=(Inst *)(p); NEXT_P0;})
#  define NEXT_INST	((Cell)cfa)
#  define INC_IP(const_inc)	({cfa=ip[const_inc]; ip+=(const_inc);})
#  define DEF_CA
#  define NEXT_P1	(ip++)
#  define NEXT_P2	({goto *cfa;})
#endif

#if THREADING_SCHEME==8
/* direct threading scheme 8: i386 hack */
#  define NEXT_P0
#  define IP		(ip)
#  define SET_IP(p)	({ip=(p); NEXT_P0;})
#  define NEXT_INST	(*IP)
#  define INC_IP(const_inc)	({ ip+=(const_inc);})
#  define DEF_CA
#  define NEXT_P1	(ip++)
#  define NEXT_P2	({goto **(ip-1);})
#  define dont_need_cfa
#endif

#if THREADING_SCHEME==9
/* direct threading scheme 9: prefetching (for PowerPC) */
/* note that the "cfa=next_cfa;" occurs only in NEXT_P1, because this
   works out better with the capabilities of gcc to introduce and
   schedule the mtctr instruction. */
#  define NEXT_P0
#  define IP		ip
#  define SET_IP(p)	({ip=(p); next_cfa=*ip; NEXT_P0;})
#  define NEXT_INST	(next_cfa)
#  define INC_IP(const_inc)	({next_cfa=IP[const_inc]; ip+=(const_inc);})
#  define DEF_CA	
#  define NEXT_P1	({cfa=next_cfa; ip++; next_cfa=*ip;})
#  define NEXT_P2	({goto *cfa;})
#  define MORE_VARS	Inst next_cfa;
#endif

#if THREADING_SCHEME==10
/* direct threading scheme 10: plain (no attempt at scheduling) */
#  define NEXT_P0
#  define IP		(ip)
#  define SET_IP(p)	({ip=(p); NEXT_P0;})
#  define NEXT_INST	(*ip)
#  define INC_IP(const_inc)	({ip+=(const_inc);})
#  define DEF_CA
#  define NEXT_P1
#  define NEXT_P2	({cfa=*ip++; goto *cfa;})
#endif


#define NEXT ({DEF_CA NEXT_P1; NEXT_P2;})
#define IPTOS NEXT_INST

#ifdef VM_PROFILING
#define SUPER_END  vm_count_block(IP)
#else
#define SUPER_END
#endif

Cell engine(Inst *ip0, Cell *sp, char *fp, Cell *procsp)
{
  /* VM registers (you may want to use gcc's "Explicit Reg Vars" here) */
  Inst * ip;
  #if !defined(dont_need_cfa)
  Inst * cfa;
  #endif

#ifdef USE_spTOS
      Cell   spTOS;
#else
# define spTOS (sp[0])
#endif

  static Inst   labels[] = {
#include "vm/gen/rb-labels.i"
  };
#ifdef MORE_VARS
  MORE_VARS
#endif

  if (ip0 == NULL) {
    vm_prim = labels;
    return 0;
  }
  if (vm_debug)
      fprintf(vm_out,"entering engine(%p,%p,%p)\n",ip0,sp,fp);

  /* I don't have a clue where these things come from,
     but I've put them in macros.h for the moment */
  IF_spTOS(spTOS = sp[0]);

  SET_IP(ip0);
  SUPER_END;  /* count the BB starting at ip0 */
  NEXT;

#include "vm/gen/rb-vm-lbl.i"
}

