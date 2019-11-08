#ifndef GRANITA_H
#define GRANITA_H
// Paul's and Dmitry's laptops have different attributes for this function in system libraries.
void abort(void) __attribute__((__cold__)) __attribute__((__noreturn__));
#include "include/ruby/encoding.h" // for rb_encoding
#include "internal.h"
#include "ruby.h"
#include "vm_core.h"
// for explanation of WTF is happening here, see ruby.h and
// https://silverhammermba.github.io/emberb/c/ and
// http://clalance.blogspot.com/2011/01/writing-ruby-extensions-in-c-part-9.html

// ****
// ****                       Internal Helper Functions
// ****

void dbg_sorbet_validate_id(ID value, char *name) __attribute__((weak)) {
    if (UNLIKELY(value == 0)) {
        printf("ERROR: %s is 0\n", name);
        abort();
    }
}

const char *dbg_pi(ID id) __attribute__((weak)) {
    return rb_id2name(id);
}

const char *dbg_p(VALUE obj) __attribute__((weak)) {
    char *ret = RSTRING_PTR(rb_sprintf("%" PRIsVALUE, obj));
    return ret;
}

// ****
// ****                       Singletons
// ****

VALUE sorbet_rubyTrue() __attribute__((always_inline)) {
    return RUBY_Qtrue;
}

VALUE sorbet_rubyFalse() __attribute__((always_inline)) {
    return RUBY_Qfalse;
}

VALUE sorbet_rubyNil() __attribute__((always_inline)) {
    return RUBY_Qnil;
}

// ****
// ****                       Conversions between Ruby values and C values
// ****
long sorbet_rubyValueToLong(VALUE val) __attribute__((always_inline)) {
    return FIX2LONG(val);
}

VALUE sorbet_longToRubyValue(long i) __attribute__((always_inline)) {
    return LONG2FIX(i);
}

double sorbet_rubyValueToDouble(VALUE val) __attribute__((always_inline)) {
    return RFLOAT_VALUE(val);
}

VALUE sorbet_doubleToRubyValue(double u) __attribute__((always_inline)) {
    return DBL2NUM(u);
}

// ****
// ****                       Integer
// ****
VALUE sorbet_Integer_plus_Integer(VALUE a, VALUE b) __attribute__((always_inline)) {
    return sorbet_longToRubyValue(sorbet_rubyValueToLong(a) + sorbet_rubyValueToLong(b));
}

VALUE sorbet_Integer_minus_Integer(VALUE a, VALUE b) __attribute__((always_inline)) {
    return sorbet_longToRubyValue(sorbet_rubyValueToLong(a) - sorbet_rubyValueToLong(b));
}

VALUE sorbet_Integer_less_Integer(VALUE a, VALUE b) __attribute__((always_inline)) {
    return (sorbet_rubyValueToLong(a) < sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

VALUE sorbet_Integer_greater_Integer(VALUE a, VALUE b) __attribute__((always_inline)) {
    return (sorbet_rubyValueToLong(a) > sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

VALUE sorbet_Integer_greatereq_Integer(VALUE a, VALUE b) __attribute__((always_inline)) {
    return (sorbet_rubyValueToLong(a) >= sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

VALUE sorbet_Integer_lesseq_Integer(VALUE a, VALUE b) __attribute__((always_inline)) {
    return (sorbet_rubyValueToLong(a) <= sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

VALUE sorbet_Integer_eq_Integer(VALUE a, VALUE b) __attribute__((always_inline)) {
    return (sorbet_rubyValueToLong(a) == sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

VALUE sorbet_Integer_neq_Integer(VALUE a, VALUE b) __attribute__((always_inline)) {
    return (sorbet_rubyValueToLong(a) != sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

// ****
// ****                       Operations on Strings
// ****
const char *sorbet_rubyStringToCPtr(VALUE value) __attribute__((always_inline)) {
    return RSTRING_PTR(value);
}

long sorbet_rubyStringLength(VALUE value) __attribute__((always_inline)) {
    return RSTRING_LEN(value);
}

VALUE sorbet_CPtrToRubyString(const char *ptr, long length) __attribute__((always_inline)) {
    return rb_str_new(ptr, length);
}

VALUE sorbet_stringPlus(VALUE str1, VALUE str2) __attribute__((always_inline)) {
    return rb_str_plus(str1, str2);
}

// ****
// ****                       Operations on Arrays
// ****
long sorbet_rubyArrayLen(VALUE array) __attribute__((always_inline)) {
    return RARRAY_LEN(array);
}

VALUE sorbet_newRubyArray(long size) __attribute__((always_inline)) {
    return rb_ary_new2(size);
}

VALUE sorbet_newRubyArrayWithElems(long size, const VALUE *elems) __attribute__((always_inline)) {
    return rb_ary_new4(size, elems);
}

void sorbet_arrayPush(VALUE array, VALUE element) __attribute__((always_inline)) {
    rb_ary_push(array, element);
}

void sorbet_arrayStore(VALUE array, long idx, VALUE value) __attribute__((always_inline)) {
    rb_ary_store(array, idx, value);
}

VALUE sorbet_arrayGet(VALUE array, long idx) __attribute__((always_inline)) {
    return rb_ary_entry(array, idx);
}

// ****
// ****                       Operations on Hashes
// ****
//
VALUE sorbet_newRubyHash() __attribute__((always_inline)) {
    return rb_hash_new();
}

void sorbet_hashStore(VALUE hash, VALUE key, VALUE value) __attribute__((always_inline)) {
    rb_hash_aset(hash, key, value);
}

VALUE sorbet_hashGet(VALUE hash, VALUE key) __attribute__((always_inline)) {
    return rb_hash_aref(hash, key);
}

// possible return values for `func`:
//  - ST_CONTINUE, then the rest of the hash is processed as normal.
//  - ST_STOP, then no further processing of the hash is done.
//  - ST_DELETE, then the current hash key is deleted from the hash and the rest
//  of the hash is processed
//  - ST_CHECK, then the hash is checked to see if it has been modified during
//  this operation. If so, processing of the hash stops.
/*
void sorbet_hashEach(VALUE hash, int(*func)(VALUE key, VALUE val,
VALUE in), VALUE closure) { return rb_hash_foreach(hash, func, closure);
}
*/

// ****
// ****                       Operations on Ruby ID's
// ****

ID sorbet_IDIntern(const char *value, long length) __attribute__((always_inline)) {
    return rb_intern2(value, length);
}

ID sorbet_symToID(VALUE sym) __attribute__((always_inline)) {
    return SYM2ID(sym);
}

ID sorbet_IDToSym(ID id) __attribute__((always_inline)) {
    dbg_sorbet_validate_id(id, "id");
    return ID2SYM(id);
}

VALUE sorbet_getRubyClassOf(VALUE value) __attribute__((always_inline)) {
    return CLASS_OF(value);
}

const char *sorbet_getRubyClassName(VALUE object) __attribute__((always_inline)) {
    return rb_obj_classname(object);
}
// ****
// ****                       Tests
// ****

_Bool sorbet_testIsTruthy(VALUE value) __attribute__((always_inline)) {
    return RB_TEST(value);
}

_Bool sorbet_testIsTrue(VALUE value) __attribute__((always_inline)) {
    return value == RUBY_Qtrue;
}

_Bool sorbet_testIsFalse(VALUE value) __attribute__((always_inline)) {
    return value == RUBY_Qfalse;
}

_Bool sorbet_testIsNil(VALUE value) __attribute__((always_inline)) {
    return value == RUBY_Qnil;
}

_Bool sorbet_testIsUndef(VALUE value) __attribute__((always_inline)) {
    return value == RUBY_Qundef;
}

_Bool sorbet_testIsSymbol(VALUE value) __attribute__((always_inline)) {
    return RB_SYMBOL_P(value);
}

_Bool sorbet_testIsFloat(VALUE value) __attribute__((always_inline)) {
    return RB_FLOAT_TYPE_P(value);
}

_Bool sorbet_testIsHash(VALUE value) __attribute__((always_inline)) {
    return TYPE(value) == RUBY_T_HASH;
}

_Bool sorbet_testIsArray(VALUE value) __attribute__((always_inline)) {
    return TYPE(value) == RUBY_T_ARRAY;
}

_Bool sorbet_testIsString(VALUE value) __attribute__((always_inline)) {
    return TYPE(value) == RUBY_T_STRING;
}

// ****
// ****                       Variables
// ****

VALUE sorbet_instanceVariableGet(VALUE receiver, ID name) __attribute__((always_inline)) {
    dbg_sorbet_validate_id(name, "name");
    return rb_ivar_get(receiver, name);
}

VALUE sorbet_instanceVariableSet(VALUE receiver, ID name, VALUE newValue) __attribute__((always_inline)) {
    dbg_sorbet_validate_id(name, "name");
    return rb_ivar_set(receiver, name, newValue);
}

VALUE sorbet_globalVariableGet(const char *name) __attribute__((always_inline)) {
    return rb_gv_get(name);
}

void sorbet_globalVariableSet(const char *name, VALUE newValue) __attribute__((always_inline)) {
    rb_gv_set(name, newValue);
}

VALUE sorbet_classVariableGet(VALUE _class, ID name) __attribute__((always_inline)) {
    dbg_sorbet_validate_id(name, "name");
    return rb_cvar_get(_class, name);
}

void sorbet_classVariableSet(VALUE _class, ID name, VALUE newValue) __attribute__((always_inline)) {
    dbg_sorbet_validate_id(name, "name");
    rb_cvar_set(_class, name, newValue);
}

// ****
// ****                       Constants, Classes and Modules
// ****

VALUE sorbet_rb_cObject() {
    return rb_cObject;
}

void sorbet_defineTopLevelConstant(const char *name, VALUE value) __attribute__((always_inline)) {
    rb_define_global_const(name, value);
}

void sorbet_defineNestedCosntant(VALUE owner, const char *name, VALUE value) __attribute__((always_inline)) {
    rb_define_const(owner, name, value);
}

RUBY_EXTERN rb_serial_t ruby_vm_global_constant_state;

long sorbet_getConstantEpoch() {
    return ruby_vm_global_constant_state;
}

VALUE sorbet_getMethodBlockAsProc() {
    if (rb_block_given_p()) {
        return rb_block_proc();
    }
    return Qnil;
}

// Trying to be a copy of rb_mod_const_get
VALUE sorbet_getConstant(const char *path, long pathLen) __attribute__((noinline)) {
    VALUE name, mod;
    rb_encoding *enc;
    const char *pbeg, *p, *pend;
    ID id;
    int recur = 1;
    int DISABLED_CODE = 0;

    id = rb_intern2(path, pathLen);
    name = ID2SYM(id);
    mod = sorbet_rb_cObject();
    enc = rb_enc_get(name);

    pbeg = p = path;
    pend = path + pathLen;

    if (DISABLED_CODE && (p >= pend || !*p)) {
    wrong_name:
        rb_raise(rb_eRuntimeError, "wrong constant name %" PRIsVALUE "%" PRIsVALUE, mod, name);
    }

    if (DISABLED_CODE && (p + 2 < pend && p[0] == ':' && p[1] == ':')) {
        mod = rb_cObject;
        p += 2;
        pbeg = p;
    }

    while (p < pend) {
        VALUE part;
        long len, beglen;

        while (p < pend && *p != ':')
            p++;

        if (pbeg == p)
            goto wrong_name;

        id = rb_check_id_cstr(pbeg, len = p - pbeg, enc);
        beglen = pbeg - path;

        if (p < pend && p[0] == ':') {
            if (p + 2 >= pend || p[1] != ':')
                goto wrong_name;
            p += 2;
            pbeg = p;
        }

        if (!RB_TYPE_P(mod, T_MODULE) && !RB_TYPE_P(mod, T_CLASS)) {
            rb_raise(rb_eTypeError, "%" PRIsVALUE " does not refer to class/module", name);
        }

        if (!id) {
            part = rb_str_subseq(name, beglen, len);
            OBJ_FREEZE(part);
            if (!rb_is_const_name(part)) {
                name = part;
                goto wrong_name;
            } else if (!rb_method_basic_definition_p(CLASS_OF(mod), idConst_missing)) {
                part = rb_str_intern(part);
                mod = rb_const_missing(mod, part);
                continue;
            } else {
                rb_mod_const_missing(mod, part);
            }
        }
        if (!rb_is_const_id(id)) {
            name = ID2SYM(id);
            goto wrong_name;
        }
        if (!recur) {
            mod = rb_const_get_at(mod, id);
        } else if (beglen == 0) {
            mod = rb_const_get(mod, id);
        } else {
            mod = rb_const_get_from(mod, id);
        }
    }

    return mod;
}
// End copy of rb_mod_const_get

void sorbet_setConstant(VALUE mod, const char *name, long nameLen, VALUE value) __attribute__((noinline)) {
    ID id = rb_intern2(name, nameLen);
    return rb_const_set(mod, id, value);
}

VALUE sorbet_defineTopLevelModule(const char *name) __attribute__((always_inline)) {
    return rb_define_module(name);
}

VALUE sorbet_defineNestedModule(VALUE owner, const char *name) __attribute__((always_inline)) {
    return rb_define_module_under(owner, name);
}

VALUE sorbet_defineTopClassOrModule(const char *name, VALUE super) __attribute__((always_inline)) {
    return rb_define_class(name, super);
}

VALUE sorbet_defineNestedClass(VALUE owner, const char *name, VALUE super) __attribute__((always_inline)) {
    return rb_define_class_under(owner, name, super);
}

// this DOES override existing methods
void sorbet_defineMethod(VALUE klass, const char *name, VALUE (*methodPtr)(ANYARGS), int argc)
    __attribute__((always_inline)) {
    rb_define_method(klass, name, methodPtr, argc);
}

// this DOES override existing methods
void sorbet_defineMethodSingleton(VALUE klass, const char *name, VALUE (*methodPtr)(ANYARGS), int argc)
    __attribute__((always_inline)) {
    rb_define_singleton_method(klass, name, methodPtr, argc);
}

// ****
// ****                       Calls
// ****

VALUE sorbet_callSuper(int argc, const VALUE *argv) __attribute__((always_inline)) {
    return rb_call_super(argc, argv);
}

VALUE sorbet_callBlock(VALUE array) __attribute__((always_inline)) {
    // TODO: one day we should use rb_yield_values, as it saves an allocation, but
    // for now, do the easy thing
    return rb_yield_splat(array);
}

VALUE sorbet_callFunc(VALUE recv, ID func, int argc, __attribute__((noescape)) const VALUE *const restrict argv)
    __attribute__((always_inline)) {
    dbg_sorbet_validate_id(func, "func");
    return rb_funcallv(recv, func, argc, argv);
}

VALUE sorbet_callFuncBlock(VALUE recv, ID func, int argc, __attribute__((noescape)) const VALUE *const restrict argv,
                           VALUE (*blockImpl)(VALUE, VALUE, int, VALUE *, VALUE), VALUE closure)
    __attribute__((always_inline)) {
    dbg_sorbet_validate_id(func, "func");
    return rb_block_call(recv, func, argc, argv, blockImpl, closure);
}

// defining a way to allocate storage for custom class:
//      VALUE allocate(VALUE klass);
//      rb_define_alloc_func(class, &allocate)
//

VALUE sorbet_rb_arity_error_new(int argc, int min, int max) {
    VALUE err_mess = 0;
    if (min == max) {
        err_mess = rb_sprintf("wrong number of arguments (given %d, expected %d)", argc, min);
    } else if (max == UNLIMITED_ARGUMENTS) {
        err_mess = rb_sprintf("wrong number of arguments (given %d, expected %d+)", argc, min);
    } else {
        err_mess = rb_sprintf("wrong number of arguments (given %d, expected %d..%d)", argc, min, max);
    }
    return rb_exc_new3(rb_eArgError, err_mess);
}

void sorbet_cast_failure(VALUE value, char *castMethod, char *type) __attribute__((__cold__))
__attribute__((__noreturn__)) {
    // TODO: cargo cult more of
    // https://github.com/sorbet/sorbet/blob/b045fb1ba12756c3760fe516dc315580d93f3621/gems/sorbet-runtime/lib/types/types/base.rb#L105
    //
    // e.g. we need to teach the `got` part to do `T.class_of`
    rb_raise(rb_eTypeError, "%s: Expected type %s, got %s with value %" PRIsVALUE, castMethod, type,
             rb_obj_classname(value), value);
}

void sorbet_rb_error_arity(int argc, int min, int max) __attribute__((__noreturn__)) {
    rb_exc_raise(sorbet_rb_arity_error_new(argc, min, max));
}

// ****
// **** Optimized versions of callFunc.
// **** Should use the same calling concention.
// **** Call it ending with `_no_type_guard` if implementation has a backed in slowpath
// ****
// ****

// ****
// **** Closures
// ****

// this specifies to use ruby default free for freeing(which is just xfree). Thus objects should be allocated with
// xmalloc

struct sorbet_Closure {
    const int size;
    VALUE closureData[]; // this is a rarely known feature of C99 https://en.wikipedia.org/wiki/Flexible_array_member
};

struct sorbet_Closure *sorbet_Closure_alloc(int elemCount) {
    return (struct sorbet_Closure *)xmalloc(sizeof(struct sorbet_Closure) + sizeof(VALUE) * elemCount);
}

void sorbet_Closure_mark(void *closurePtr) {
    // this might be possible to make more efficient using rb_mark_tbl
    struct sorbet_Closure *ptr = (struct sorbet_Closure *)closurePtr;
    rb_gc_mark_values(ptr->size, &ptr->closureData[0]);
}

size_t sorbet_Closure_size(const void *closurePtr) __attribute__((const)) {
    // this might be possible to make more efficient using rb_mark_tbl
    struct sorbet_Closure *ptr = (struct sorbet_Closure *)closurePtr;
    return sizeof(struct sorbet_Closure) + ptr->size * sizeof(VALUE);
}

const rb_data_type_t closureInfo = {
    "CompiledClosure", // this shouldn't ever be visible to users
    {
        /* mark = */ sorbet_Closure_mark,
        /* free = */ RUBY_DEFAULT_FREE,  // this uses xfree and optimzies it
        /* size = */ sorbet_Closure_size /*, compact */
    },
    /* parent = */ NULL,
    /* arbitrary data = */ NULL,
    /* flags = */ RUBY_TYPED_FREE_IMMEDIATELY /* deferred free */,
};

VALUE sorbet_allocClosureAsValue(int elemCount) {
    struct sorbet_Closure *ptr = sorbet_Closure_alloc(elemCount);
    return TypedData_Wrap_Struct(rb_cData, &closureInfo, ptr);
}

VALUE *sorbet_getClosureElem(VALUE closure, int elemId) {
    struct sorbet_Closure *ptr = (struct sorbet_Closure *)RTYPEDDATA_DATA(closure);
    return &(ptr->closureData[elemId]);
}

// ****
// **** Implementation helpers for type tests
// ****

_Bool sorbet_isa_Integer(VALUE obj) __attribute__((const)) {
    return RB_FIXNUM_P(obj);
}

_Bool sorbet_isa_TrueClass(VALUE obj) __attribute__((const)) {
    return obj == RUBY_Qtrue;
}

_Bool sorbet_isa_FalseClass(VALUE obj) __attribute__((const)) {
    return obj == RUBY_Qfalse;
}

_Bool sorbet_isa_NilClass(VALUE obj) __attribute__((const)) {
    return obj == RUBY_Qnil;
}

_Bool sorbet_isa_Symbol(VALUE obj) __attribute__((const)) {
    return RB_SYMBOL_P(obj);
}

_Bool sorbet_isa_Float(VALUE obj) __attribute__((const)) {
    return RB_FLOAT_TYPE_P(obj);
}

_Bool sorbet_isa_Untyped(VALUE obj) __attribute__((const)) {
    return 1;
}

_Bool sorbet_isa_Hash(VALUE obj) __attribute__((const)) {
    return RB_TYPE_P(obj, T_HASH);
}

_Bool sorbet_isa_Array(VALUE obj) __attribute__((const)) {
    return RB_TYPE_P(obj, T_ARRAY);
}

_Bool sorbet_isa_Regexp(VALUE obj) __attribute__((const)) {
    return RB_TYPE_P(obj, T_REGEXP);
}

_Bool sorbet_isa_Rational(VALUE obj) __attribute__((const)) {
    return RB_TYPE_P(obj, T_RATIONAL);
}

_Bool sorbet_isa_String(VALUE obj) __attribute__((const)) {
    return RB_TYPE_P(obj, T_STRING);
}

/*
_Bool sorbet_isa_Method(VALUE obj) __attribute__((const))  {
    return rb_obj_is_method(obj) == Qtrue;
}
*/

_Bool sorbet_isa_Proc(VALUE obj) __attribute__((const)) {
    return rb_obj_is_proc(obj) == Qtrue;
}

_Bool sorbet_isa(VALUE obj, VALUE class) __attribute__((const)) {
    return rb_obj_is_kind_of(obj, class) == Qtrue;
}

_Bool sorbet_isa_class_of(VALUE obj, VALUE class) __attribute__((const)) {
    return (obj == class) || (rb_obj_is_kind_of(obj, rb_cModule) && rb_class_inherited_p(obj, class));
}

// Intrinsics. See CallCMethod in SymbolIntrinsics.cc

void sorbet_ensure_arity(int argc, int expected) {
    if (argc != expected) {
        sorbet_rb_error_arity(argc, expected, expected);
    }
}

VALUE sorbet_boolToRuby(_Bool b) {
    if (b) {
        return RUBY_Qtrue;
    }
    return RUBY_Qfalse;
}

VALUE sorbet_rb_array_len(VALUE recv, ID func, int argc, const VALUE *const restrict argv) {
    sorbet_ensure_arity(argc, 0);
    return sorbet_longToRubyValue(rb_array_len(recv));
}

// TODO: add many from https://github.com/ruby/ruby/blob/ruby_2_6/include/ruby/intern.h#L55
VALUE sorbet_rb_int_plus(VALUE recv, int argc, const VALUE *const restrict argv) {
    sorbet_ensure_arity(argc, 1);
    VALUE y = argv[0];
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            return rb_fix_plus_fix(recv, y);
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_plus(y, recv);
        } else if (RB_TYPE_P(recv, T_FLOAT)) {
            return DBL2NUM((double)FIX2LONG(recv) + RFLOAT_VALUE(y));
        } else if (RB_TYPE_P(y, T_COMPLEX)) {
            return rb_complex_plus(y, recv);
        }
        // fall through to coerce
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_plus(recv, y);
    }
    return rb_num_coerce_bin(recv, y, '+');
}

VALUE sorbet_rb_int_minus(VALUE recv, int argc, const VALUE *const restrict argv) {
    sorbet_ensure_arity(argc, 1);
    // optimized version from numeric.c
    VALUE y = argv[0];
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            return rb_fix_minus_fix(recv, y);
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            VALUE x = rb_int2big(FIX2LONG(recv));
            return rb_big_minus(x, y);
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return DBL2NUM((double)FIX2LONG(recv) - RFLOAT_VALUE(y));
        }
        // fall throught to coerece
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_minus(recv, y);
    }
    return rb_num_coerce_bin(recv, y, '-');
}

VALUE sorbet_rb_int_gt(VALUE recv, int argc, const VALUE *const restrict argv) {
    sorbet_ensure_arity(argc, 1);
    return rb_int_gt(recv, argv[0]);
}

VALUE sorbet_rb_int_lt(VALUE recv, int argc, const VALUE *const restrict argv) {
    sorbet_ensure_arity(argc, 1);
    VALUE y = argv[0];
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) < FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_cmp(y, recv) == INT2FIX(+1) ? Qtrue : Qfalse;
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return rb_integer_float_cmp(recv, y) == INT2FIX(-1) ? Qtrue : Qfalse;
        } else {
            return rb_num_coerce_relop(recv, y, '<');
        }
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_lt(recv, y);
    }
    return Qnil;
}

VALUE sorbet_rb_int_equal(VALUE recv, int argc, const VALUE *const restrict argv) {
    sorbet_ensure_arity(argc, 1);
    return rb_int_equal(recv, argv[0]);
}

VALUE sorbet_rb_int_neq(VALUE recv, int argc, const VALUE *const restrict argv) {
    sorbet_ensure_arity(argc, 1);
    return sorbet_boolToRuby(rb_int_equal(recv, argv[0]) == sorbet_rubyFalse());
}

// ****
// ****                       Intrinsics
// ****

VALUE sorbet_splatIntrinsic(VALUE arr, VALUE before, VALUE after) {
    long len = sorbet_rubyArrayLen(arr);
    int size = sorbet_rubyValueToLong(before) + sorbet_rubyValueToLong(after);
    if (len < size) {
        VALUE newArr = rb_ary_dup(arr);
        for (int i = 0; i < size; i++) {
            sorbet_arrayPush(newArr, sorbet_rubyNil());
        }
        return newArr;
    }
    return arr;
}

#endif
