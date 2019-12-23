#ifndef SORBET_LLVM_IMPORTED_INTRINSICS_H
#define SORBET_LLVM_IMPORTED_INTRINSICS_H

#include "ruby.h"

// Array#initialize_copy
// Array#replace
// Calling convention: 1
extern VALUE rb_ary_replace(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_replace(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_replace(recv, arg_0);
}

// Array#[]
// Array#slice
// Calling convention: -1
extern VALUE rb_ary_aref(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_ary_aref(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_ary_aref(argc, args, recv);
}

// Array#at
// Calling convention: 1
extern VALUE rb_ary_at(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_at(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_at(recv, arg_0);
}

// Array#last
// Calling convention: -1
extern VALUE rb_ary_last(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_ary_last(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_ary_last(argc, args, recv);
}

// Array#<<
// Calling convention: 1
extern VALUE rb_ary_push(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_push(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_push(recv, arg_0);
}

// Array#each
// Calling convention: 0
extern VALUE rb_ary_each(VALUE obj);

VALUE sorbet_int_rb_ary_each(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_ary_each(recv);
}

// Array#sort
// Calling convention: 0
extern VALUE rb_ary_sort(VALUE obj);

VALUE sorbet_int_rb_ary_sort(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_ary_sort(recv);
}

// Array#sort!
// Calling convention: 0
extern VALUE rb_ary_sort_bang(VALUE obj);

VALUE sorbet_int_rb_ary_sort_bang(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_ary_sort_bang(recv);
}

// Array#delete
// Calling convention: 1
extern VALUE rb_ary_delete(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_delete(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_delete(recv, arg_0);
}

// Array#clear
// Calling convention: 0
extern VALUE rb_ary_clear(VALUE obj);

VALUE sorbet_int_rb_ary_clear(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_ary_clear(recv);
}

// Array#include?
// Calling convention: 1
extern VALUE rb_ary_includes(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_includes(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_includes(recv, arg_0);
}

// Array#<=>
// Calling convention: 1
extern VALUE rb_ary_cmp(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_cmp(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_cmp(recv, arg_0);
}

// Array#assoc
// Calling convention: 1
extern VALUE rb_ary_assoc(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_assoc(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_assoc(recv, arg_0);
}

// Array#rassoc
// Calling convention: 1
extern VALUE rb_ary_rassoc(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_rassoc(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_rassoc(recv, arg_0);
}

// Array#+
// Calling convention: 1
extern VALUE rb_ary_plus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_ary_plus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_ary_plus(recv, arg_0);
}

// Complex#real
// Calling convention: 0
extern VALUE rb_complex_real(VALUE obj);

VALUE sorbet_int_rb_complex_real(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_complex_real(recv);
}

// Complex#imaginary
// Complex#imag
// Calling convention: 0
extern VALUE rb_complex_imag(VALUE obj);

VALUE sorbet_int_rb_complex_imag(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_complex_imag(recv);
}

// Complex#-@
// Calling convention: 0
extern VALUE rb_complex_uminus(VALUE obj);

VALUE sorbet_int_rb_complex_uminus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_complex_uminus(recv);
}

// Complex#+
// Calling convention: 1
extern VALUE rb_complex_plus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_complex_plus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_complex_plus(recv, arg_0);
}

// Complex#-
// Calling convention: 1
extern VALUE rb_complex_minus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_complex_minus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_complex_minus(recv, arg_0);
}

// Complex#*
// Calling convention: 1
extern VALUE rb_complex_mul(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_complex_mul(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_complex_mul(recv, arg_0);
}

// Complex#/
// Calling convention: 1
extern VALUE rb_complex_div(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_complex_div(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_complex_div(recv, arg_0);
}

// Complex#**
// Calling convention: 1
extern VALUE rb_complex_pow(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_complex_pow(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_complex_pow(recv, arg_0);
}

// Complex#abs
// Complex#magnitude
// Calling convention: 0
extern VALUE rb_complex_abs(VALUE obj);

VALUE sorbet_int_rb_complex_abs(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_complex_abs(recv);
}

// Complex#arg
// Complex#angle
// Complex#phase
// Calling convention: 0
extern VALUE rb_complex_arg(VALUE obj);

VALUE sorbet_int_rb_complex_arg(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_complex_arg(recv);
}

// Complex#conjugate
// Complex#conj
// Calling convention: 0
extern VALUE rb_complex_conjugate(VALUE obj);

VALUE sorbet_int_rb_complex_conjugate(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_complex_conjugate(recv);
}

// Hash#rehash
// Calling convention: 0
extern VALUE rb_hash_rehash(VALUE obj);

VALUE sorbet_int_rb_hash_rehash(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_rehash(recv);
}

// Hash#[]
// Calling convention: 1
extern VALUE rb_hash_aref(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_hash_aref(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_hash_aref(recv, arg_0);
}

// Hash#[]=
// Hash#store
// Calling convention: 2
extern VALUE rb_hash_aset(VALUE obj, VALUE arg_0, VALUE arg_1);

VALUE sorbet_int_rb_hash_aset(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 2, 2);
    VALUE arg_0 = args[0];
    VALUE arg_1 = args[1];
    return rb_hash_aset(recv, arg_0, arg_1);
}

// Hash#default_proc=
// Calling convention: 1
extern VALUE rb_hash_set_default_proc(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_hash_set_default_proc(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_hash_set_default_proc(recv, arg_0);
}

// Hash#size
// Hash#length
// Calling convention: 0
extern VALUE rb_hash_size(VALUE obj);

VALUE sorbet_int_rb_hash_size(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_size(recv);
}

// Hash#keys
// Calling convention: 0
extern VALUE rb_hash_keys(VALUE obj);

VALUE sorbet_int_rb_hash_keys(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_keys(recv);
}

// Hash#values
// Calling convention: 0
extern VALUE rb_hash_values(VALUE obj);

VALUE sorbet_int_rb_hash_values(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_values(recv);
}

// Hash#values_at
// Calling convention: -1
extern VALUE rb_hash_values_at(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_hash_values_at(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_hash_values_at(argc, args, recv);
}

// Hash#fetch_values
// Calling convention: -1
extern VALUE rb_hash_fetch_values(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_hash_fetch_values(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_hash_fetch_values(argc, args, recv);
}

// Hash#delete_if
// Calling convention: 0
extern VALUE rb_hash_delete_if(VALUE obj);

VALUE sorbet_int_rb_hash_delete_if(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_delete_if(recv);
}

// Hash#keep_if
// Calling convention: 0
extern VALUE rb_hash_keep_if(VALUE obj);

VALUE sorbet_int_rb_hash_keep_if(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_keep_if(recv);
}

// Hash#select
// Hash#filter
// Calling convention: 0
extern VALUE rb_hash_select(VALUE obj);

VALUE sorbet_int_rb_hash_select(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_select(recv);
}

// Hash#select!
// Hash#filter!
// Calling convention: 0
extern VALUE rb_hash_select_bang(VALUE obj);

VALUE sorbet_int_rb_hash_select_bang(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_select_bang(recv);
}

// Hash#reject
// Calling convention: 0
extern VALUE rb_hash_reject(VALUE obj);

VALUE sorbet_int_rb_hash_reject(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_reject(recv);
}

// Hash#reject!
// Calling convention: 0
extern VALUE rb_hash_reject_bang(VALUE obj);

VALUE sorbet_int_rb_hash_reject_bang(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_reject_bang(recv);
}

// Hash#clear
// Calling convention: 0
extern VALUE rb_hash_clear(VALUE obj);

VALUE sorbet_int_rb_hash_clear(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_clear(recv);
}

// Hash#assoc
// Calling convention: 1
extern VALUE rb_hash_assoc(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_hash_assoc(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_hash_assoc(recv, arg_0);
}

// Hash#rassoc
// Calling convention: 1
extern VALUE rb_hash_rassoc(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_hash_rassoc(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_hash_rassoc(recv, arg_0);
}

// Hash#include?
// Hash#member?
// Hash#has_key?
// Hash#key?
// Calling convention: 1
extern VALUE rb_hash_has_key(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_hash_has_key(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_hash_has_key(recv, arg_0);
}

// Hash#compare_by_identity?
// Calling convention: 0
extern VALUE rb_hash_compare_by_id_p(VALUE obj);

VALUE sorbet_int_rb_hash_compare_by_id_p(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_hash_compare_by_id_p(recv);
}

// Integer#odd?
// Calling convention: 0
extern VALUE rb_int_odd_p(VALUE obj);

VALUE sorbet_int_rb_int_odd_p(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_int_odd_p(recv);
}

// Integer#<=>
// Calling convention: 1
extern VALUE rb_int_cmp(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_cmp(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_cmp(recv, arg_0);
}

// Integer#-@
// Calling convention: 0
extern VALUE rb_int_uminus(VALUE obj);

VALUE sorbet_int_rb_int_uminus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_int_uminus(recv);
}

// Integer#+
// Calling convention: 1
extern VALUE rb_int_plus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_plus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_plus(recv, arg_0);
}

// Integer#-
// Calling convention: 1
extern VALUE rb_int_minus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_minus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_minus(recv, arg_0);
}

// Integer#*
// Calling convention: 1
extern VALUE rb_int_mul(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_mul(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_mul(recv, arg_0);
}

// Integer#/
// Calling convention: 1
extern VALUE rb_int_div(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_div(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_div(recv, arg_0);
}

// Integer#div
// Calling convention: 1
extern VALUE rb_int_idiv(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_idiv(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_idiv(recv, arg_0);
}

// Integer#%
// Integer#modulo
// Calling convention: 1
extern VALUE rb_int_modulo(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_modulo(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_modulo(recv, arg_0);
}

// Integer#divmod
// Calling convention: 1
extern VALUE rb_int_divmod(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_divmod(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_divmod(recv, arg_0);
}

// Integer#fdiv
// Calling convention: 1
extern VALUE rb_int_fdiv(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_fdiv(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_fdiv(recv, arg_0);
}

// Integer#**
// Calling convention: 1
extern VALUE rb_int_pow(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_pow(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_pow(recv, arg_0);
}

// Integer#pow
// Calling convention: -1
extern VALUE rb_int_powm(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_int_powm(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_int_powm(argc, args, recv);
}

// Integer#abs
// Integer#magnitude
// Calling convention: 0
extern VALUE rb_int_abs(VALUE obj);

VALUE sorbet_int_rb_int_abs(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_int_abs(recv);
}

// Integer#===
// Integer#==
// Calling convention: 1
extern VALUE rb_int_equal(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_equal(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_equal(recv, arg_0);
}

// Integer#>
// Calling convention: 1
extern VALUE rb_int_gt(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_gt(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_gt(recv, arg_0);
}

// Integer#>=
// Calling convention: 1
extern VALUE rb_int_ge(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_ge(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_ge(recv, arg_0);
}

// Integer#&
// Calling convention: 1
extern VALUE rb_int_and(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_and(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_and(recv, arg_0);
}

// Integer#<<
// Calling convention: 1
extern VALUE rb_int_lshift(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_int_lshift(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_int_lshift(recv, arg_0);
}

// Float#-@
// Calling convention: 0
extern VALUE rb_float_uminus(VALUE obj);

VALUE sorbet_int_rb_float_uminus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_float_uminus(recv);
}

// Float#+
// Calling convention: 1
extern VALUE rb_float_plus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_float_plus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_float_plus(recv, arg_0);
}

// Float#*
// Calling convention: 1
extern VALUE rb_float_mul(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_float_mul(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_float_mul(recv, arg_0);
}

// Float#**
// Calling convention: 1
extern VALUE rb_float_pow(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_float_pow(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_float_pow(recv, arg_0);
}

// Float#>
// Calling convention: 1
extern VALUE rb_float_gt(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_float_gt(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_float_gt(recv, arg_0);
}

// Float#abs
// Float#magnitude
// Calling convention: 0
extern VALUE rb_float_abs(VALUE obj);

VALUE sorbet_int_rb_float_abs(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_float_abs(recv);
}

// Float#infinite?
// Calling convention: 0
extern VALUE rb_flo_is_infinite_p(VALUE obj);

VALUE sorbet_int_rb_flo_is_infinite_p(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_flo_is_infinite_p(recv);
}

// Float#finite?
// Calling convention: 0
extern VALUE rb_flo_is_finite_p(VALUE obj);

VALUE sorbet_int_rb_flo_is_finite_p(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_flo_is_finite_p(recv);
}

// Integer#gcd
// Calling convention: 1
extern VALUE rb_gcd(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_gcd(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_gcd(recv, arg_0);
}

// Integer#lcm
// Calling convention: 1
extern VALUE rb_lcm(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_lcm(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_lcm(recv, arg_0);
}

// Integer#gcdlcm
// Calling convention: 1
extern VALUE rb_gcdlcm(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_gcdlcm(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_gcdlcm(recv, arg_0);
}
#endif /* SORBET_LLVM_IMPORTED_INTRINSICS_H */
