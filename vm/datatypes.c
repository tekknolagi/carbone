/* datatype definitions for the vm

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


#include <stdio.h>
#include "vm/vm.h"
#include "comp/cfunc.h"


// generate arguments

void genarg_i(Inst **vmcodepp, Cell i)
{
  *((Cell *) *vmcodepp) = i;
  (*vmcodepp)++;
}

void genarg_target(Inst **vmcodepp, Inst *target)
{
  *((Inst **) *vmcodepp) = target;
  (*vmcodepp)++;
}

void genarg_cfunc(Inst **vmcodepp, void *cfunc)
{
  *((Inst **) *vmcodepp) = cfunc;
  (*vmcodepp)++;
}



// trace execution

void hex_diff(void *v, void *old, char* s);

void trace_insn(char *name, Inst *_ip, char *fp, Cell *sp) {
  static Inst *oip = (void*)0xffffffff; 
  static Cell *osp = (void*)0xffffffff;
  static char *ofp = (void*)0xffffffff;

  if (vm_debug) {
    Inst  *ip = _ip-1;
    char  ip_s[20], fp_s[20], sp_s[20], buf[100];
    
    hex_diff(ip, oip, ip_s);
    hex_diff(fp, ofp, fp_s);
    hex_diff(sp, osp, sp_s);

    sprintf(buf, "%s: %-12s fp=%s sp=%s", ip_s, name, fp_s, sp_s);
    oip = ip;  ofp = fp;  osp = sp;
    fprintf(vm_out, "%s", buf);
  }
}

void hex_diff(void *v, void *old, char* s) {
  char      vs[20], os[20];
  int       i, diff=0;

  sprintf(vs, "%10p", v);
  sprintf(os, "%10p", old);

  for (diff = 0; vs[diff]==os[diff] && diff < 20; diff++);
  diff -= 2;
  //if (diff < 0) diff = 0;


  for (i = 0; vs[i] != 0 && i < 20; i++) {
    if (i == diff)
      s[i] = '_';
    else if (i> diff)
      s[i] = vs[i];
    else
      s[i] = ' ';
  }
  s[i] = 0;
}







// print arguments

void printarg_i(Cell i)
{
  fprintf(vm_out, "0x%lx ", i);
  if (is_symbol((VALUE)i))
    fprintf(vm_out, "(%s)", sym2str(i));
  else if (i_is_a(T_CONSTANT, i)) {
    if (i == rb_false) fprintf(vm_out, "(false)");
    if (i == rb_true) fprintf(vm_out, "(true)");
    if (i == rb_nil) fprintf(vm_out, "(nil)");
  }
  fprintf(vm_out, " ");
}

void printarg_target(Inst *target)
{
  fprintf(vm_out, "%p ", target);
}


void printarg_cfunc( void *cfunc)
{
  List *l = cfunc_list;
  Cfunc *f = 0;
  while (l) {
    f = (Cfunc*)l->head;
    if (f->func == cfunc) break;
    l = l->rest;
  }
  if (f && f->func == cfunc)
    fprintf(vm_out, "%s ", sym2str(f->name));
  else
    fprintf(vm_out, "%p ", cfunc);
}

void printarg_sel(Sym sel) {
  fprintf(vm_out, "%s<0x%lx> ", sym_sym2str(sel), sel);
}


void printarg_meth(Method *m) {
  if (m == 0)
    fprintf(vm_out, "(nil) ");
  else
    fprintf(vm_out, "%s#%s ",
	    sym2str(m->recv_class->name),
	    sym2str(m->name));
}


void printarg_class(Class *c) {
  if (c == 0)
    fprintf(vm_out, "(nil) ");
  else
    fprintf(vm_out, "%s ", sym2str(c->name));
}

#include "fond/class.h"

void printarg_rcv(VALUE rcv) {
  Class *c = class_of(rcv);

  if (c == 0)
    fprintf(vm_out, "<>");
  else
    fprintf(vm_out, "<%s>", sym2str(c->name));

  printarg_i(rcv);
}
