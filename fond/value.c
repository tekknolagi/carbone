/* value converters that may be inlined but don't have to

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

#include "fond/value.h"



//TODO:  change representation of negative fixnums to facilitate operations

MAY_BE_INLINED VALUE long2value(long x) {
  if (x >= 0)
    return (VALUE)(   x  | (T_FIXNUM<<imm_type_bits));
  else
    return (VALUE)( (-x) | ((T_FIXNUM<<1|1)<<(imm_type_bits-1)));
}

MAY_BE_INLINED long value2long(VALUE x) {
  #if T_FIXNUM == 0
    if (x & fixnum_sign_mask)
      return -(long)(x ^ fixnum_sign_mask);  // not better..
    else
      return (long)(x);
  #else
    if (x & fixnum_sign_mask)
      return -(long)(x & ((value_imm_type_max>>1)-1));
    else
      return (long)(x & ((value_imm_type_max>>1)-1));
  #endif
}


/*Class* ary_imm_class = {
  0,   // fixnum
  0,   // symbol
  0,   // constant
  0
};
*/
