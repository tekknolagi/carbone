
#if !defined(VALUE_H)
#define VALUE_H


typedef unsigned long VALUE;

#define ptr(x) ((VALUE)(x))   // make value of pointer
#define iptr(x) ((int*)(x))   // make *int of value
#define cptr(x) ((char*)(x))  // make *char of value

#if !defined(MAY_BE_INLINED)
#define MAY_BE_INLINED
#endif




/* types of immediate objects */
#define T_FIXNUM 0
#define T_SYMBOL 1
#define T_CONSTANT 2   // true false nil

#define imm_bits_for_type 2

#define NUM_IMM_VALUES 3  // number of immediate types


// [FIXME] True, False, Nil !
#define m_class_of_imm(x) (ary_imm_class[i_type(x)])


#define is_fixnum(x)  (i_is_a(T_FIXNUM,x))
//#define is_fixnum(x)  ("is a fixnum" && i_is_a(T_FIXNUM,x))
#define is_symbol(x)  (i_is_a(T_SYMBOL,x))



#define linux_value_allocation_scheme_1_arch_x86_32

/******************************************************
 *              value allocations for immediate objects
 */

#ifdef linux_value_allocation_scheme_1_arch_x86_32

   // have a look at /proc/<some-pid>/maps and tests/basics/test-imm-value.c
#  define imm_free_bits      27

//#  undef imm_free_bits  // doing some debugging...
//#  define imm_free_bits      16

#  define imm_type_bits      (imm_free_bits-imm_bits_for_type)

#  define value_imm_max      (1<<imm_free_bits)
#  define value_imm_type_max (1<<imm_type_bits)

#  define is_immediate(x) (((VALUE)x) < value_imm_max)

#  define i_is_a(imm_type, value) \
        (((VALUE)(value) >> imm_type_bits) == (imm_type))


#  define i_type(value) ((int)((value)>>imm_type_bits))


#  define idx2value(type, x) \
          ((VALUE)((type<<imm_type_bits) + (x)))

#  define value2idx(x) \
          ((long)(((1<<imm_type_bits)-1) & (x)))

//-------
// Fixnum
#  define fixnum_negative_mask (((ulong)~0) << imm_type_bits) // e.g. 0xff800000
#  define fixnum_sign_mask (1 << (imm_type_bits-1)) // e.g. 0x00400000

MAY_BE_INLINED VALUE long2value(long x);
MAY_BE_INLINED long value2long(VALUE x);

//-----------
// Constants:

#  define rb_true ((T_CONSTANT<<imm_type_bits)|1)
#  define rb_false ((T_CONSTANT<<imm_type_bits)|2)
#  define rb_nil ((T_CONSTANT<<imm_type_bits)|3)
#  define rb_undef ((T_CONSTANT<<imm_type_bits)|4)

// return values for iterators
// (not allowed as reference!)
#  define IT_CONTINUE ((T_CONSTANT<<imm_type_bits)|16)
#  define IT_SUCCESS  ((T_CONSTANT<<imm_type_bits)|17)
#  define IT_FAILURE  ((T_CONSTANT<<imm_type_bits)|18)
// #  define IT_OMIT_SUBTREE ((T_CONSTANT<<imm_type_bits)|19)


#endif



//-----------
// Symbol

#  define str2sym(x) idx2value(T_SYMBOL, sym_add(x))
#  define sym2str(x) sym_to_str(value2idx(x))
#  define sym2idx(x) value2idx(x)


/******************************************************
 *                                     indirect objects
 */


#define is_indirect(x) (((VALUE)x) >= value_imm_max)



#endif /* VALUE_H */



/*******************  consistency  checks  */

#if !defined(NO_CONSISTENCY_CHECKS)

#include <assert.h>
#define assert_can_be_fixnum(x) \
 if ((x >= (1ul<<(imm_type_bits-1)))) { \
   printf("problems converting %li to fixnum.", (long)(x));}

#else

#define assert_is_fixnum(x)


#endif


