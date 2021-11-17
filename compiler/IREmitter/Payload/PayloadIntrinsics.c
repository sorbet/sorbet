#ifndef SORBET_COMPILER_IMPORTED_INTRINSICS_H
#define SORBET_COMPILER_IMPORTED_INTRINSICS_H

// This file is autogenerated. Do not edit it by hand. Regenerate it with:
//   cd compiler/IREmitter/Intrinsics && make

#include "ruby.h"

typedef VALUE (*BlockFFIType)(VALUE firstYieldedArg, VALUE closure, int argCount, const VALUE *args, VALUE blockArg);

// Array#+
// Calling convention: 1
extern VALUE rb_ary_plus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_plus(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_plus(recv, arg_0);
}

// Array#<<
// Calling convention: 1
extern VALUE rb_ary_push(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_push(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_push(recv, arg_0);
}

// Array#<=>
// Calling convention: 1
extern VALUE rb_ary_cmp(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_cmp(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_cmp(recv, arg_0);
}

// Array#[]
// Array#slice
// Calling convention: -1
extern VALUE rb_ary_aref(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_ary_aref(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    return rb_ary_aref(argc, args, recv);
}

// Array#assoc
// Calling convention: 1
extern VALUE rb_ary_assoc(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_assoc(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_assoc(recv, arg_0);
}

// Array#at
// Calling convention: 1
extern VALUE rb_ary_at(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_at(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_at(recv, arg_0);
}

// Array#clear
// Calling convention: 0
extern VALUE rb_ary_clear(VALUE obj);

VALUE sorbet_int_rb_ary_clear(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_ary_clear(recv);
}

// Array#delete
// Calling convention: 1
extern VALUE rb_ary_delete(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_delete(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_delete(recv, arg_0);
}

// Array#first
// Calling convention: -1
extern VALUE sorbet_rb_ary_first(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_ary_first(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    return sorbet_rb_ary_first(argc, args, recv);
}

// Array#include?
// Calling convention: 1
extern VALUE rb_ary_includes(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_includes(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                 VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_includes(recv, arg_0);
}

// Array#initialize_copy
// Array#replace
// Calling convention: 1
extern VALUE rb_ary_replace(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_replace(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_replace(recv, arg_0);
}

// Array#join
// Calling convention: -1
extern VALUE sorbet_rb_ary_join_m(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_ary_join_m(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    return sorbet_rb_ary_join_m(argc, args, recv);
}

// Array#last
// Calling convention: -1
extern VALUE rb_ary_last(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_ary_last(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    return rb_ary_last(argc, args, recv);
}

// Array#rassoc
// Calling convention: 1
extern VALUE rb_ary_rassoc(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_rassoc(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_rassoc(recv, arg_0);
}

// Array#sort
// Calling convention: 0
extern VALUE rb_ary_sort(VALUE obj);

VALUE sorbet_int_rb_ary_sort(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_ary_sort(recv);
}

// Array#sort!
// Calling convention: 0
extern VALUE rb_ary_sort_bang(VALUE obj);

VALUE sorbet_int_rb_ary_sort_bang(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                  VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_ary_sort_bang(recv);
}

// Float#*
// Calling convention: 1
extern VALUE rb_float_mul(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_float_mul(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_float_mul(recv, arg_0);
}

// Float#**
// Calling convention: 1
extern VALUE rb_float_pow(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_float_pow(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_float_pow(recv, arg_0);
}

// Float#+
// Calling convention: 1
extern VALUE rb_float_plus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_float_plus(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_float_plus(recv, arg_0);
}

// Float#-@
// Calling convention: 0
extern VALUE rb_float_uminus(VALUE obj);

VALUE sorbet_int_rb_float_uminus(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                 VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_float_uminus(recv);
}

// Float#<
// Calling convention: 1
extern VALUE sorbet_flo_lt(VALUE obj, VALUE arg_0);

VALUE sorbet_int_flo_lt(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return sorbet_flo_lt(recv, arg_0);
}

// Float#<=
// Calling convention: 1
extern VALUE sorbet_flo_le(VALUE obj, VALUE arg_0);

VALUE sorbet_int_flo_le(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return sorbet_flo_le(recv, arg_0);
}

// Float#>
// Calling convention: 1
extern VALUE rb_float_gt(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_float_gt(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_float_gt(recv, arg_0);
}

// Float#>=
// Calling convention: 1
extern VALUE sorbet_flo_ge(VALUE obj, VALUE arg_0);

VALUE sorbet_int_flo_ge(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return sorbet_flo_ge(recv, arg_0);
}

// Float#abs
// Float#magnitude
// Calling convention: 0
extern VALUE rb_float_abs(VALUE obj);

VALUE sorbet_int_rb_float_abs(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_float_abs(recv);
}

// Float#finite?
// Calling convention: 0
extern VALUE rb_flo_is_finite_p(VALUE obj);

VALUE sorbet_int_rb_flo_is_finite_p(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                    VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_flo_is_finite_p(recv);
}

// Float#infinite?
// Calling convention: 0
extern VALUE rb_flo_is_infinite_p(VALUE obj);

VALUE sorbet_int_rb_flo_is_infinite_p(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                      VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_flo_is_infinite_p(recv);
}

// Hash#fetch
// Calling convention: -1
extern VALUE sorbet_rb_hash_fetch_m(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_hash_fetch_m(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                 VALUE closure) {
    return sorbet_rb_hash_fetch_m(argc, args, recv);
}

// Hash#include?
// Hash#member?
// Hash#has_key?
// Hash#key?
// Calling convention: 1
extern VALUE rb_hash_has_key(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_hash_has_key(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                 VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_hash_has_key(recv, arg_0);
}

// Integer#%
// Integer#modulo
// Calling convention: 1
extern VALUE rb_int_modulo(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_modulo(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_modulo(recv, arg_0);
}

// Integer#&
// Calling convention: 1
extern VALUE rb_int_and(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_and(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_and(recv, arg_0);
}

// Integer#*
// Calling convention: 1
extern VALUE rb_int_mul(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_mul(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_mul(recv, arg_0);
}

// Integer#**
// Calling convention: 1
extern VALUE rb_int_pow(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_pow(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_pow(recv, arg_0);
}

// Integer#+
// Calling convention: 1
extern VALUE rb_int_plus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_plus(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_plus(recv, arg_0);
}

// Integer#-
// Calling convention: 1
extern VALUE rb_int_minus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_minus(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_minus(recv, arg_0);
}

// Integer#-@
// Calling convention: 0
extern VALUE rb_int_uminus(VALUE obj);

VALUE sorbet_int_rb_int_uminus(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_int_uminus(recv);
}

// Integer#/
// Calling convention: 1
extern VALUE rb_int_div(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_div(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_div(recv, arg_0);
}

// Integer#<<
// Calling convention: 1
extern VALUE rb_int_lshift(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_lshift(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_lshift(recv, arg_0);
}

// Integer#<=>
// Calling convention: 1
extern VALUE rb_int_cmp(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_cmp(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_cmp(recv, arg_0);
}

// Integer#===
// Integer#==
// Calling convention: 1
extern VALUE rb_int_equal(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_equal(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_equal(recv, arg_0);
}

// Integer#>
// Calling convention: 1
extern VALUE rb_int_gt(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_gt(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_gt(recv, arg_0);
}

// Integer#>=
// Calling convention: 1
extern VALUE rb_int_ge(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_ge(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_ge(recv, arg_0);
}

// Integer#abs
// Integer#magnitude
// Calling convention: 0
extern VALUE rb_int_abs(VALUE obj);

VALUE sorbet_int_rb_int_abs(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_int_abs(recv);
}

// Integer#div
// Calling convention: 1
extern VALUE rb_int_idiv(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_idiv(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_idiv(recv, arg_0);
}

// Integer#divmod
// Calling convention: 1
extern VALUE rb_int_divmod(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_divmod(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_divmod(recv, arg_0);
}

// Integer#fdiv
// Calling convention: 1
extern VALUE rb_int_fdiv(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_fdiv(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_fdiv(recv, arg_0);
}

// Integer#gcd
// Calling convention: 1
extern VALUE rb_gcd(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_gcd(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_gcd(recv, arg_0);
}

// Integer#gcdlcm
// Calling convention: 1
extern VALUE rb_gcdlcm(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_gcdlcm(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_gcdlcm(recv, arg_0);
}

// Integer#lcm
// Calling convention: 1
extern VALUE rb_lcm(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_lcm(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_lcm(recv, arg_0);
}

// Integer#odd?
// Calling convention: 0
extern VALUE rb_int_odd_p(VALUE obj);

VALUE sorbet_int_rb_int_odd_p(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_int_odd_p(recv);
}

// Integer#pow
// Calling convention: -1
extern VALUE rb_int_powm(int const argc, VALUE *const argv, VALUE const num);

VALUE sorbet_int_rb_int_powm(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    return rb_int_powm(argc, args, recv);
}

// Regexp#encoding
// String#encoding
// Calling convention: 0
extern VALUE rb_obj_encoding(VALUE obj);

VALUE sorbet_int_rb_obj_encoding(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                 VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_encoding(recv);
}

// String#*
// Calling convention: 1
extern VALUE rb_str_times(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_times(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_times(recv, arg_0);
}

// String#+
// Calling convention: 1
extern VALUE rb_str_plus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_plus(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_plus(recv, arg_0);
}

// String#<<
// Calling convention: 1
extern VALUE rb_str_concat(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_concat(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_concat(recv, arg_0);
}

// String#==
// String#===
// Calling convention: 1
extern VALUE rb_str_equal(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_equal(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                              VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_equal(recv, arg_0);
}

// String#[]
// String#slice
// Calling convention: -1
extern VALUE sorbet_rb_str_aref_m(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_str_aref_m(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    return sorbet_rb_str_aref_m(argc, args, recv);
}

// String#dump
// Calling convention: 0
extern VALUE rb_str_dump(VALUE obj);

VALUE sorbet_int_rb_str_dump(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_str_dump(recv);
}

// String#eql?
// Calling convention: 1
extern VALUE rb_str_eql(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_eql(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_eql(recv, arg_0);
}

// String#freeze
// Calling convention: 0
extern VALUE rb_str_freeze(VALUE obj);

VALUE sorbet_int_rb_str_freeze(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_str_freeze(recv);
}

// String#initialize_copy
// String#replace
// Calling convention: 1
extern VALUE rb_str_replace(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_replace(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                VALUE closure) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_replace(recv, arg_0);
}

// String#inspect
// Calling convention: 0
extern VALUE rb_str_inspect(VALUE obj);

VALUE sorbet_int_rb_str_inspect(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                                VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_str_inspect(recv);
}

// String#intern
// String#to_sym
// Calling convention: 0
extern VALUE rb_str_intern(VALUE obj);

VALUE sorbet_int_rb_str_intern(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_str_intern(recv);
}

// String#length
// String#size
// Calling convention: 0
extern VALUE rb_str_length(VALUE obj);

VALUE sorbet_int_rb_str_length(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_str_length(recv);
}

// String#ord
// Calling convention: 0
extern VALUE rb_str_ord(VALUE obj);

VALUE sorbet_int_rb_str_ord(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_str_ord(recv);
}

// String#succ
// String#next
// Calling convention: 0
extern VALUE rb_str_succ(VALUE obj);

VALUE sorbet_int_rb_str_succ(VALUE recv, ID fun, int argc, VALUE *const restrict args, BlockFFIType blk,
                             VALUE closure) {
    rb_check_arity(argc, 0, 0);
    return rb_str_succ(recv);
}
#endif /* SORBET_COMPILER_IMPORTED_INTRINSICS_H */
