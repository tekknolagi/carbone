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

#include "vm/vm.h"

void vm_disassemble(Inst *ip, Inst *endp, Inst prim[])
{
  while (ip<endp) {
    fprintf(vm_out,"%p: ",ip);
#include "vm/gen/rb-disasm.i"
    {
      fprintf(vm_out,"unknown instruction %p",ip[0]);
      ip++;
    }
  _endif_:
    fputc('\n',vm_out);
  }
}
