#ifndef SORBET_LLVM_IMPORTED_INTRINSICS_H
#define SORBET_LLVM_IMPORTED_INTRINSICS_H

#include "ruby.h"

// BasicObject#==
// BasicObject#equal?
// Kernel#eql?
// Module#==
// Calling convention: 1
extern VALUE rb_obj_equal(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_obj_equal(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_obj_equal(recv, arg_0);
}

// BasicObject#!
// Calling convention: 0
extern VALUE rb_obj_not(VALUE obj);

VALUE sorbet_int_rb_obj_not(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_not(recv);
}

// BasicObject#!=
// Calling convention: 1
extern VALUE rb_obj_not_equal(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_obj_not_equal(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_obj_not_equal(recv, arg_0);
}

// Kernel#===
// NilClass#===
// Calling convention: 1
extern VALUE rb_equal(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_equal(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_equal(recv, arg_0);
}

// Kernel#hash
// Calling convention: 0
extern VALUE rb_obj_hash(VALUE obj);

VALUE sorbet_int_rb_obj_hash(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_hash(recv);
}

// Kernel#class
// Calling convention: 0
extern VALUE rb_obj_class(VALUE obj);

VALUE sorbet_int_rb_obj_class(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_class(recv);
}

// Kernel#dup
// Calling convention: 0
extern VALUE rb_obj_dup(VALUE obj);

VALUE sorbet_int_rb_obj_dup(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_dup(recv);
}

// Kernel#initialize_copy
// Calling convention: 1
extern VALUE rb_obj_init_copy(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_obj_init_copy(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_obj_init_copy(recv, arg_0);
}

// Kernel#initialize_dup
// Kernel#initialize_clone
// Calling convention: 1
extern VALUE rb_obj_init_dup_clone(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_obj_init_dup_clone(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_obj_init_dup_clone(recv, arg_0);
}

// Kernel#taint
// Calling convention: 0
extern VALUE rb_obj_taint(VALUE obj);

VALUE sorbet_int_rb_obj_taint(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_taint(recv);
}

// Kernel#tainted?
// Calling convention: 0
extern VALUE rb_obj_tainted(VALUE obj);

VALUE sorbet_int_rb_obj_tainted(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_tainted(recv);
}

// Kernel#untaint
// Calling convention: 0
extern VALUE rb_obj_untaint(VALUE obj);

VALUE sorbet_int_rb_obj_untaint(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_untaint(recv);
}

// Kernel#untrust
// Calling convention: 0
extern VALUE rb_obj_untrust(VALUE obj);

VALUE sorbet_int_rb_obj_untrust(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_untrust(recv);
}

// Kernel#untrusted?
// Calling convention: 0
extern VALUE rb_obj_untrusted(VALUE obj);

VALUE sorbet_int_rb_obj_untrusted(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_untrusted(recv);
}

// Kernel#trust
// Calling convention: 0
extern VALUE rb_obj_trust(VALUE obj);

VALUE sorbet_int_rb_obj_trust(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_trust(recv);
}

// Kernel#freeze
// Calling convention: 0
extern VALUE rb_obj_freeze(VALUE obj);

VALUE sorbet_int_rb_obj_freeze(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_freeze(recv);
}

// Kernel#frozen?
// Calling convention: 0
extern VALUE rb_obj_frozen_p(VALUE obj);

VALUE sorbet_int_rb_obj_frozen_p(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_frozen_p(recv);
}

// Kernel#to_s
// Calling convention: 0
extern VALUE rb_any_to_s(VALUE obj);

VALUE sorbet_int_rb_any_to_s(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_any_to_s(recv);
}

// Kernel#methods
// Calling convention: -1
extern VALUE rb_obj_methods(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_obj_methods(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_obj_methods(argc, args, recv);
}

// Kernel#singleton_methods
// Calling convention: -1
extern VALUE rb_obj_singleton_methods(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_obj_singleton_methods(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_obj_singleton_methods(argc, args, recv);
}

// Kernel#protected_methods
// Calling convention: -1
extern VALUE rb_obj_protected_methods(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_obj_protected_methods(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_obj_protected_methods(argc, args, recv);
}

// Kernel#private_methods
// Calling convention: -1
extern VALUE rb_obj_private_methods(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_obj_private_methods(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_obj_private_methods(argc, args, recv);
}

// Kernel#public_methods
// Calling convention: -1
extern VALUE rb_obj_public_methods(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_obj_public_methods(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_obj_public_methods(argc, args, recv);
}

// Kernel#instance_variables
// Calling convention: 0
extern VALUE rb_obj_instance_variables(VALUE obj);

VALUE sorbet_int_rb_obj_instance_variables(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_instance_variables(recv);
}

// Kernel#instance_of?
// Calling convention: 1
extern VALUE rb_obj_is_instance_of(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_obj_is_instance_of(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_obj_is_instance_of(recv, arg_0);
}

// Kernel#kind_of?
// Kernel#is_a?
// Calling convention: 1
extern VALUE rb_obj_is_kind_of(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_obj_is_kind_of(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_obj_is_kind_of(recv, arg_0);
}

// Kernel#tap
// Calling convention: 0
extern VALUE rb_obj_tap(VALUE obj);

VALUE sorbet_int_rb_obj_tap(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_tap(recv);
}

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

// String#initialize_copy
// String#replace
// Calling convention: 1
extern VALUE rb_str_replace(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_replace(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_replace(recv, arg_0);
}

// String#==
// String#===
// Calling convention: 1
extern VALUE rb_str_equal(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_equal(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_equal(recv, arg_0);
}

// String#eql?
// Calling convention: 1
extern VALUE rb_str_eql(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_eql(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_eql(recv, arg_0);
}

// String#+
// Calling convention: 1
extern VALUE rb_str_plus(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_plus(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_plus(recv, arg_0);
}

// String#*
// Calling convention: 1
extern VALUE rb_str_times(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_times(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_times(recv, arg_0);
}

// String#length
// String#size
// Calling convention: 0
extern VALUE rb_str_length(VALUE obj);

VALUE sorbet_int_rb_str_length(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_str_length(recv);
}

// String#succ
// String#next
// Calling convention: 0
extern VALUE rb_str_succ(VALUE obj);

VALUE sorbet_int_rb_str_succ(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_str_succ(recv);
}

// String#freeze
// Calling convention: 0
extern VALUE rb_str_freeze(VALUE obj);

VALUE sorbet_int_rb_str_freeze(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_str_freeze(recv);
}

// String#inspect
// Calling convention: 0
extern VALUE rb_str_inspect(VALUE obj);

VALUE sorbet_int_rb_str_inspect(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_str_inspect(recv);
}

// String#dump
// Calling convention: 0
extern VALUE rb_str_dump(VALUE obj);

VALUE sorbet_int_rb_str_dump(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_str_dump(recv);
}

// String#<<
// Calling convention: 1
extern VALUE rb_str_concat(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_str_concat(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_str_concat(recv, arg_0);
}

// String#intern
// String#to_sym
// Calling convention: 0
extern VALUE rb_str_intern(VALUE obj);

VALUE sorbet_int_rb_str_intern(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_str_intern(recv);
}

// String#ord
// Calling convention: 0
extern VALUE rb_str_ord(VALUE obj);

VALUE sorbet_int_rb_str_ord(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_str_ord(recv);
}

// String#encoding
// Calling convention: 0
extern VALUE rb_obj_encoding(VALUE obj);

VALUE sorbet_int_rb_obj_encoding(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_encoding(recv);
}

// BasicObject#instance_eval
// Calling convention: -1
extern VALUE rb_obj_instance_eval(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_obj_instance_eval(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_obj_instance_eval(argc, args, recv);
}

// BasicObject#instance_exec
// Calling convention: -1
extern VALUE rb_obj_instance_exec(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_obj_instance_exec(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_obj_instance_exec(argc, args, recv);
}

// BasicObject#__send__
// Kernel#send
// Calling convention: -1
extern VALUE rb_f_send(int argc, const VALUE *args, VALUE obj);

VALUE sorbet_int_rb_f_send(VALUE recv, int argc, VALUE *const restrict args) {
    return rb_f_send(argc, args, recv);
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

// Kernel#method
// Calling convention: 1
extern VALUE rb_obj_method(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_obj_method(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_obj_method(recv, arg_0);
}

// Kernel#public_method
// Calling convention: 1
extern VALUE rb_obj_public_method(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_obj_public_method(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_obj_public_method(recv, arg_0);
}

// Kernel#singleton_method
// Calling convention: 1
extern VALUE rb_obj_singleton_method(VALUE obj, VALUE arg_0);

VALUE sorbet_int_rb_obj_singleton_method(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 1, 1);
    VALUE arg_0 = args[0];
    return rb_obj_singleton_method(recv, arg_0);
}

// BasicObject#__id__
// Kernel#object_id
// Calling convention: 0
extern VALUE rb_obj_id(VALUE obj);

VALUE sorbet_int_rb_obj_id(VALUE recv, int argc, VALUE *const restrict args) {
    rb_check_arity(argc, 0, 0);
    return rb_obj_id(recv);
}
#endif /* SORBET_LLVM_IMPORTED_INTRINSICS_H */
