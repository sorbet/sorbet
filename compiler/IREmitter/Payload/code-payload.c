#include "sorbet_version/sorbet_version.h"

// These are public Ruby headers. Feel free to add more from the include/ruby
// directory
#include "ruby/encoding.h" // for rb_encoding

// These are special "public" headers which don't live in include/ruby for some
// reason
#include "internal.h"
#include "ruby.h"

// This is probably a bad idea but is needed for so many things
#include "vm_core.h"

// This is for the enum definition for YARV instructions
#include "insns.inc"

#define SORBET_ATTRIBUTE(...) __attribute__((__VA_ARGS__))
#define SORBET_INLINE __attribute__((always_inline))

typedef VALUE (*BlockFFIType)(VALUE firstYieldedArg, VALUE closure, int argCount, VALUE *args, VALUE blockArg);
typedef VALUE (*ExceptionFFIType)(VALUE **pc, VALUE *iseq_encoded, VALUE closure);

// compiler is closely aware of layout of this struct
struct FunctionInlineCache {
    const rb_callable_method_entry_t *me;

    // used to compare for validity:
    // https://github.com/ruby/ruby/blob/97d75639a9970ce3868ba91a57be1856a3957711/vm_method.c#L822-L823
    rb_serial_t method_state;
    rb_serial_t class_serial;
};

struct sorbet_iterMethodArg {
    VALUE recv;
    ID func;
    int argc;
    const VALUE *argv;
    int kw_splat;
    struct FunctionInlineCache *cache;
};

// Functions known to the compiler.
//
// We have to be a little tricky here, as LLVM will eliminate declarations that are unused
// from the emitted IR.  So we use the following macro to ensure that our declarations are
// "used"; the placeholder function will be eliminated by other optimizations.
#define SORBET_ALIVE(rettype, name, rest)                                 \
    extern rettype name rest;                                             \
    VALUE sorbet_exists_to_keep_alive_##name() __attribute__((optnone)) { \
        return (long)&name;                                               \
    }

SORBET_ALIVE(const char *, sorbet_dbg_p, (VALUE obj));
SORBET_ALIVE(void, sorbet_stopInDebugger, (void));

SORBET_ALIVE(void, sorbet_cast_failure,
             (VALUE value, char *castMethod, char *type) __attribute__((__cold__, __noreturn__)));
SORBET_ALIVE(void, sorbet_raiseArity, (int argc, int min, int max) __attribute__((__noreturn__)));
SORBET_ALIVE(void, sorbet_raiseExtraKeywords, (VALUE hash) __attribute__((__noreturn__)));
SORBET_ALIVE(VALUE, sorbet_t_absurd, (VALUE val) __attribute__((__cold__)));

SORBET_ALIVE(void *, sorbet_allocateRubyStackFrame,
             (VALUE funcName, ID func, VALUE filename, VALUE realpath, unsigned char *parent, int iseqType,
              int startline, int endline, ID *locals, int numLocals));
SORBET_ALIVE(VALUE, sorbet_getConstant, (const char *path, long pathLen));
SORBET_ALIVE(VALUE, sorbet_setConstant, (VALUE mod, const char *name, long nameLen, VALUE value));

SORBET_ALIVE(const VALUE, sorbet_readRealpath, (void));
SORBET_ALIVE(void, sorbet_popRubyStack, (void));

SORBET_ALIVE(void, sorbet_vm_env_write_slowpath, (const VALUE *, int, VALUE));
SORBET_ALIVE(void, sorbet_inlineCacheInvalidated, (VALUE recv, struct FunctionInlineCache *cache, ID mid));
SORBET_ALIVE(VALUE, sorbet_callFuncWithCache,
             (VALUE recv, ID func, int argc, SORBET_ATTRIBUTE(noescape) const VALUE *const restrict argv, int kw_splat,
              struct FunctionInlineCache *cache));

SORBET_ALIVE(void, sorbet_setMethodStackFrame,
             (rb_execution_context_t * ec, rb_control_frame_t *cfp, const rb_iseq_t *iseq));
SORBET_ALIVE(void, sorbet_setExceptionStackFrame,
             (rb_execution_context_t * ec, rb_control_frame_t *cfp, const rb_iseq_t *iseq));

SORBET_ALIVE(VALUE, sorbet_blockReturnUndef, (VALUE * *pc, VALUE *iseq_encoded, VALUE closure));

SORBET_ALIVE(VALUE, sorbet_splatIntrinsic,
             (VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk, VALUE closure));
SORBET_ALIVE(VALUE, sorbet_definedIntrinsic,
             (VALUE recv, ID fun, int argc, const VALUE *const restrict, BlockFFIType blk, VALUE closure));
SORBET_ALIVE(VALUE, sorbet_stringInterpolate,
             (VALUE recv, ID fun, int argc, const VALUE *const restrict, BlockFFIType blk, VALUE closure));

SORBET_ALIVE(VALUE, sorbet_rb_array_square_br_slowpath,
             (VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk, VALUE closure));

SORBET_ALIVE(VALUE, sorbet_rb_int_plus_slowpath, (VALUE, VALUE));
SORBET_ALIVE(VALUE, sorbet_rb_int_minus_slowpath, (VALUE, VALUE));

SORBET_ALIVE(VALUE, sorbet_rb_int_lt_slowpath, (VALUE, VALUE));
SORBET_ALIVE(VALUE, sorbet_rb_int_gt_slowpath, (VALUE, VALUE));
SORBET_ALIVE(VALUE, sorbet_rb_int_le_slowpath, (VALUE, VALUE));
SORBET_ALIVE(VALUE, sorbet_rb_int_ge_slowpath, (VALUE, VALUE));

SORBET_ALIVE(VALUE, sorbet_i_getRubyClass, (const char *const className, long classNameLen) __attribute__((const)));
SORBET_ALIVE(VALUE, sorbet_i_getRubyConstant, (const char *const className, long classNameLen) __attribute__((const)));

// The next several functions exist to convert Ruby definitions into LLVM IR, and
// are always inlined as a consequence.

// ****
// ****                       Singletons
// ****

SORBET_INLINE
VALUE sorbet_rubyTrue() {
    return RUBY_Qtrue;
}

SORBET_INLINE
VALUE sorbet_rubyFalse() {
    return RUBY_Qfalse;
}

SORBET_INLINE
VALUE sorbet_rubyNil() {
    return RUBY_Qnil;
}

// use this undefined value when you have a variable that should _never_ escape to ruby.
SORBET_INLINE
VALUE sorbet_rubyUndef() {
    return RUBY_Qundef;
}

SORBET_INLINE
SORBET_ATTRIBUTE(pure)
VALUE sorbet_rubyTopSelf() {
    return GET_VM()->top_self;
}

// ****
// ****                       Implementation helpers for type tests
// ****

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Integer(VALUE obj) {
    return RB_FIXNUM_P(obj);
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_TrueClass(VALUE obj) {
    return obj == RUBY_Qtrue;
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_FalseClass(VALUE obj) {
    return obj == RUBY_Qfalse;
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_NilClass(VALUE obj) {
    return obj == RUBY_Qnil;
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Symbol(VALUE obj) {
    return RB_SYMBOL_P(obj);
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Float(VALUE obj) {
    return RB_FLOAT_TYPE_P(obj);
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Untyped(VALUE obj) {
    return 1;
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Hash(VALUE obj) {
    return RB_TYPE_P(obj, T_HASH);
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Array(VALUE obj) {
    return RB_TYPE_P(obj, T_ARRAY);
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Regexp(VALUE obj) {
    return RB_TYPE_P(obj, T_REGEXP);
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Rational(VALUE obj) {
    return RB_TYPE_P(obj, T_RATIONAL);
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_String(VALUE obj) {
    return RB_TYPE_P(obj, T_STRING);
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Proc(VALUE obj) {
    return rb_obj_is_proc(obj) == Qtrue;
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_RootSingleton(VALUE obj) {
    return obj == GET_VM()->top_self;
}

SORBET_ATTRIBUTE(const) VALUE rb_obj_is_kind_of(VALUE, VALUE);
SORBET_ATTRIBUTE(const) VALUE rb_class_inherited_p(VALUE, VALUE);

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa(VALUE obj, VALUE class) {
    return rb_obj_is_kind_of(obj, class) == Qtrue;
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_class_of(VALUE obj, VALUE class) {
    return (obj == class) || (rb_obj_is_kind_of(obj, rb_cModule) && rb_class_inherited_p(obj, class));
}

SORBET_INLINE
int sorbet_rubyIseqTypeMethod() {
    return ISEQ_TYPE_METHOD;
}

SORBET_INLINE
int sorbet_rubyIseqTypeBlock() {
    return ISEQ_TYPE_BLOCK;
}

SORBET_INLINE
int sorbet_rubyIseqTypeRescue() {
    return ISEQ_TYPE_RESCUE;
}

SORBET_INLINE
int sorbet_rubyIseqTypeEnsure() {
    return ISEQ_TYPE_ENSURE;
}

RUBY_EXTERN rb_serial_t ruby_vm_global_constant_state;

SORBET_INLINE
rb_serial_t sorbet_getConstantEpoch() {
    return ruby_vm_global_constant_state;
}

SORBET_INLINE
VALUE sorbet_getMethodBlockAsProc() {
    if (rb_block_given_p()) {
        return rb_block_proc();
    }
    return Qnil;
}

SORBET_INLINE
VALUE sorbet_defineTopLevelModule(const char *name) {
    return rb_define_module(name);
}

SORBET_INLINE
VALUE sorbet_defineNestedModule(VALUE owner, const char *name) {
    return rb_define_module_under(owner, name);
}

SORBET_INLINE
VALUE sorbet_defineTopClassOrModule(const char *name, VALUE super) {
    return rb_define_class(name, super);
}

SORBET_INLINE
VALUE sorbet_defineNestedClass(VALUE owner, const char *name, VALUE super) {
    return rb_define_class_under(owner, name, super);
}

// this DOES override existing methods
SORBET_INLINE
void sorbet_defineMethod(VALUE klass, const char *name, VALUE (*methodPtr)(ANYARGS), int argc) {
    rb_define_method(klass, name, methodPtr, argc);
}

// this DOES override existing methods
SORBET_INLINE
void sorbet_defineMethodSingleton(VALUE klass, const char *name, VALUE (*methodPtr)(ANYARGS), int argc) {
    rb_define_singleton_method(klass, name, methodPtr, argc);
}

// ****
// ****                       Variables
// ****

SORBET_INLINE
VALUE sorbet_globalVariableGet(ID name) {
    return rb_gvar_get(rb_global_entry(name));
}

SORBET_INLINE
void sorbet_globalVariableSet(ID name, VALUE newValue) {
    rb_gvar_set(rb_global_entry(name), newValue);
}

SORBET_INLINE
VALUE sorbet_instanceVariableGet(VALUE receiver, ID name) {
    return rb_ivar_get(receiver, name);
}

SORBET_INLINE
VALUE sorbet_instanceVariableSet(VALUE receiver, ID name, VALUE newValue) {
    return rb_ivar_set(receiver, name, newValue);
}

SORBET_INLINE
VALUE sorbet_classVariableGet(VALUE _class, ID name) {
    return rb_cvar_get(_class, name);
}

SORBET_INLINE
void sorbet_classVariableSet(VALUE _class, ID name, VALUE newValue) {
    rb_cvar_set(_class, name, newValue);
}

// ****
// ****                       Operations on Ruby ID's
// ****

SORBET_INLINE
ID sorbet_idIntern(const char *value, long length) {
    return rb_intern2(value, length);
}

SORBET_INLINE
ID sorbet_symToID(VALUE sym) {
    return SYM2ID(sym);
}

SORBET_INLINE
VALUE sorbet_IDToSym(ID id) {
    return ID2SYM(id);
}

SORBET_INLINE
VALUE sorbet_getRubyClassOf(VALUE value) {
    return CLASS_OF(value);
}

SORBET_INLINE
const char *sorbet_getRubyClassName(VALUE object) {
    return rb_obj_classname(object);
}

// ****
// ****                       Conversions between Ruby values and C values
// ****

SORBET_INLINE
long sorbet_rubyValueToLong(VALUE val) {
    return FIX2LONG(val);
}

SORBET_INLINE
VALUE sorbet_longToRubyValue(long i) {
    return LONG2FIX(i);
}

SORBET_INLINE
double sorbet_rubyValueToDouble(VALUE val) {
    return RFLOAT_VALUE(val);
}

SORBET_INLINE
VALUE sorbet_doubleToRubyValue(double u) {
    return DBL2NUM(u);
}

// ****
// ****                       Operations on Arrays
// ****

SORBET_INLINE
int sorbet_rubyArrayLen(VALUE array) {
    return RARRAY_LEN(array);
}

SORBET_INLINE
const VALUE *sorbet_rubyArrayInnerPtr(VALUE array) {
    // there's also a transient version of this function if we ever decide to want more speed. transient stands for that
    // we _should not_ allow to execute any code between getting these pointers and reading elements from
    return RARRAY_CONST_PTR(array);
}

SORBET_INLINE
VALUE sorbet_newRubyArray(long size) {
    return rb_ary_new2(size);
}

SORBET_INLINE
VALUE sorbet_newRubyArrayWithElems(long size, const VALUE *elems) {
    return rb_ary_new4(size, elems);
}

SORBET_INLINE
void sorbet_arrayPush(VALUE array, VALUE element) {
    rb_ary_push(array, element);
}

// ****
// ****                       Operations on Hashes
// ****

SORBET_INLINE
VALUE sorbet_newRubyHash() {
    return rb_hash_new();
}

SORBET_INLINE
VALUE sorbet_hashBuild(int argc, const VALUE *argv) {
    VALUE ret = rb_hash_new_with_size(argc / 2);
    if (argc > 0) {
        // We can use rb_hash_bulk_insert here because rb_hash_new_with_size freshly allocates.
        // We have tried in the past to use rb_hash_bulk_insert after clearing an existing hash,
        // and things broke wonderfully, because Ruby Hash objects are either backed by a small (<8 element)
        // or large hash table implementation, and neither Hash#clear nor rb_hash_bulk_insert changes
        // what kind of Hash object it is.
        rb_hash_bulk_insert(argc, argv, ret);
    }
    return ret;
}

SORBET_INLINE
VALUE sorbet_hashDup(VALUE hash) {
    return rb_hash_dup(hash);
}

SORBET_INLINE
void sorbet_hashStore(VALUE hash, VALUE key, VALUE value) {
    rb_hash_aset(hash, key, value);
}

SORBET_INLINE
VALUE sorbet_hashGet(VALUE hash, VALUE key) {
    return rb_hash_aref(hash, key);
}

SORBET_INLINE
void sorbet_hashUpdate(VALUE hash, VALUE other) {
    // TODO(trevor) inline a definition of `merge!` here
    rb_funcall(hash, rb_intern2("merge!", 6), 1, other);
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
// ****                       Operations on Strings
// ****

SORBET_INLINE
const char *sorbet_rubyStringToCPtr(VALUE value) {
    return RSTRING_PTR(value);
}

SORBET_INLINE
long sorbet_rubyStringLength(VALUE value) {
    return RSTRING_LEN(value);
}

SORBET_INLINE
VALUE sorbet_cPtrToRubyString(const char *ptr, long length) {
    return rb_str_new(ptr, length);
}

SORBET_INLINE
VALUE sorbet_cPtrToRubyStringFrozen(const char *ptr, long length) {
    VALUE ret = rb_fstring_new(ptr, length);
    rb_gc_register_mark_object(ret);
    return ret;
}

SORBET_INLINE
VALUE sorbet_cPtrToRubyRegexpFrozen(const char *ptr, long length, int options) {
    VALUE ret = rb_reg_new(ptr, length, options);
    rb_gc_register_mark_object(ret);
    return ret;
}

SORBET_INLINE
VALUE sorbet_stringPlus(VALUE str1, VALUE str2) {
    return rb_str_plus(str1, str2);
}

// ****
// ****                       Tests
// ****

SORBET_INLINE
_Bool sorbet_testIsTruthy(VALUE value) {
    return RB_TEST(value);
}

SORBET_INLINE
_Bool sorbet_testIsUndef(VALUE value) {
    return value == RUBY_Qundef;
}

// https://ruby-doc.org/core-2.6.3/Object.html#method-i-eql-3F
SORBET_INLINE
_Bool sorbet_testObjectEqual_p(VALUE obj1, VALUE obj2) {
    return obj1 == obj2;
}

SORBET_INLINE
void sorbet_ensure_arity(int argc, int expected) {
    if (UNLIKELY(argc != expected)) {
        sorbet_raiseArity(argc, expected, expected);
    }
}

SORBET_INLINE
void sorbet_checkStack() {
    // This is actually pretty slow. We should probably use guard pages instead.
    ruby_stack_check();
}

SORBET_INLINE
VALUE sorbet_boolToRuby(_Bool b) {
    if (b) {
        return RUBY_Qtrue;
    }
    return RUBY_Qfalse;
}

// TODO: add many from https://github.com/ruby/ruby/blob/ruby_2_6/include/ruby/intern.h#L55
VALUE sorbet_T_unsafe(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk, VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    return argv[0];
}

VALUE sorbet_T_must(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk, VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    if (UNLIKELY(argv[0] == Qnil)) {
        rb_raise(rb_eTypeError, "Passed `nil` into T.must");
    } else {
        return argv[0];
    }
}

VALUE sorbet_rb_array_len(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                          VALUE closure) {
    sorbet_ensure_arity(argc, 0);
    return LONG2FIX(rb_array_len(recv));
}

SORBET_INLINE
VALUE sorbet_rb_array_square_br(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                VALUE closure) {
    VALUE ary = recv;
    rb_check_arity(argc, 1, 2);
    if (LIKELY(argc == 1)) {
        VALUE arg = argv[0];
        if (LIKELY(FIXNUM_P(arg))) {
            return rb_ary_entry(ary, FIX2LONG(arg));
        }
    }
    return sorbet_rb_array_square_br_slowpath(recv, fun, argc, argv, blk, closure);
}

// This is an adjusted version of the intrinsic from the ruby vm. The major change is that instead of handling the case
// where a range is used as the key, we defer back to the VM.
// https://github.com/ruby/ruby/blob/ruby_2_6/array.c#L1980-L2005
SORBET_INLINE
VALUE sorbet_rb_array_square_br_eq(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                   VALUE closure) {
    long offset, beg, len;

    if (UNLIKELY(argc == 3)) {
        goto range;
    }
    rb_check_arity(argc, 2, 2);
    rb_check_frozen(recv);
    if (LIKELY(FIXNUM_P(argv[0]))) {
        offset = FIX2LONG(argv[0]);
        goto fixnum;
    }
    if (UNLIKELY(rb_range_beg_len(argv[0], &beg, &len, RARRAY_LEN(recv), 1))) {
    range:
        return rb_funcallv(recv, fun, argc, argv);
    }
    offset = NUM2LONG(argv[0]);
fixnum:
    rb_ary_store(recv, offset, argv[1]);
    return argv[1];
}

SORBET_INLINE
VALUE sorbet_rb_array_empty(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                            VALUE closure) {
    rb_check_arity(argc, 0, 0);
    if (RARRAY_LEN(recv) == 0) {
        return Qtrue;
    }
    return Qfalse;
}

SORBET_INLINE
VALUE sorbet_rb_hash_square_br(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 1, 1);
    return rb_hash_aref(recv, argv[0]);
}

SORBET_INLINE
VALUE sorbet_rb_hash_square_br_eq(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                  VALUE closure) {
    rb_check_arity(argc, 2, 2);
    return rb_hash_aset(recv, argv[0], argv[1]);
}

SORBET_INLINE
VALUE sorbet_rb_int_plus(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                         VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    VALUE y = argv[0];
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            return rb_fix_plus_fix(recv, y);
        }
    }
    return sorbet_rb_int_plus_slowpath(recv, y);
}

SORBET_INLINE
VALUE sorbet_rb_int_minus(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                          VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    // optimized version from numeric.c
    VALUE y = argv[0];
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            return rb_fix_minus_fix(recv, y);
        }
    }
    return sorbet_rb_int_minus_slowpath(recv, y);
}

SORBET_INLINE
VALUE sorbet_rb_int_mul(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                        VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    return rb_int_mul(recv, argv[0]);
}

SORBET_INLINE
VALUE sorbet_rb_int_div(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                        VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    return rb_int_div(recv, argv[0]);
}

SORBET_INLINE
VALUE sorbet_rb_int_lt(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                       VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    VALUE y = argv[0];
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) < FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        }
    }
    return sorbet_rb_int_lt_slowpath(recv, y);
}

SORBET_INLINE
VALUE sorbet_rb_int_gt(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                       VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    VALUE y = argv[0];
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) > FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        }
    }
    return sorbet_rb_int_gt_slowpath(recv, y);
}

SORBET_INLINE
VALUE sorbet_rb_int_le(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                       VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    VALUE y = argv[0];
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) <= FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        }
    }
    return sorbet_rb_int_le_slowpath(recv, y);
}

SORBET_INLINE
VALUE sorbet_rb_int_ge(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                       VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    VALUE y = argv[0];
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) >= FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        }
    }
    return sorbet_rb_int_ge_slowpath(recv, y);
}

SORBET_INLINE
VALUE sorbet_rb_int_equal(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                          VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    return rb_int_equal(recv, argv[0]);
}

SORBET_INLINE
VALUE sorbet_rb_int_neq(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                        VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    return sorbet_boolToRuby(rb_int_equal(recv, argv[0]) == RUBY_Qfalse);
}

SORBET_INLINE
VALUE sorbet_rb_int_to_s(VALUE x, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk, VALUE closure) {
    int base;

    rb_check_arity(argc, 0, 1);
    if (argc == 1) {
        base = NUM2INT(argv[0]);
    } else {
        base = 10;
    }
    if (LIKELY(FIXNUM_P(x))) {
        return rb_fix2str(x, base);
    }
    if (RB_TYPE_P(x, T_BIGNUM)) {
        return rb_big2str(x, base);
    }

    return rb_any_to_s(x);
}

// ****
// ****                       Name Based Intrinsics
// ****

SORBET_INLINE
VALUE sorbet_buildHashIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                VALUE closure) {
    return sorbet_hashBuild(argc, argv);
}

SORBET_INLINE
VALUE sorbet_buildArrayIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                 VALUE closure) {
    if (argc == 0) {
        return rb_ary_new();
    }
    return rb_ary_new_from_values(argc, argv);
}

SORBET_INLINE
VALUE sorbet_buildRangeIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                 VALUE closure) {
    sorbet_ensure_arity(argc, 3);

    VALUE start = argv[0];
    VALUE end = argv[1];
    VALUE excludeEnd = argv[2];
    return rb_range_new(start, end, excludeEnd);
}

SORBET_INLINE
VALUE sorbet_int_bool_true(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                           VALUE closure) {
    return Qtrue;
}

SORBET_INLINE
VALUE sorbet_int_bool_false(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                            VALUE closure) {
    return Qfalse;
}

SORBET_INLINE
VALUE sorbet_int_bool_and(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                          VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    if (argv[0] != Qnil && argv[0] != Qfalse) {
        return Qtrue;
    }
    return Qfalse;
}

SORBET_INLINE
VALUE sorbet_int_bool_nand(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                           VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    if (argv[0] != Qnil && argv[0] != Qfalse) {
        return Qfalse;
    }
    return Qtrue;
}

SORBET_INLINE
VALUE sorbet_bang(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk, VALUE closure) {
    // RTEST is false when the value is Qfalse or Qnil
    if (RTEST(recv)) {
        if (recv == Qtrue) {
            return Qfalse;
        } else {
            // slow path - dispatch via the VM
            return rb_funcallv(recv, rb_intern("!"), argc, argv);
        }
    } else {
        return Qtrue;
    }
}

SORBET_INLINE
VALUE sorbet_selfNew(VALUE recv, ID fun, int argc, VALUE *argv, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, UNLIMITED_ARGUMENTS);
    VALUE obj = argv[0];
    return rb_funcallv(obj, rb_intern("new"), argc - 1, argv + 1);
}

// ****
// ****                       Calls
// ****

SORBET_INLINE
VALUE sorbet_getKWArg(VALUE maybeHash, VALUE key) {
    if (maybeHash == RUBY_Qundef) {
        return RUBY_Qundef;
    }

    // TODO: ruby seems to do something smarter here:
    //  https://github.com/ruby/ruby/blob/5aa0e6bee916f454ecf886252e1b025d824f7bd8/class.c#L1901
    //
    return rb_hash_delete_entry(maybeHash, key);
}

SORBET_INLINE
VALUE sorbet_assertNoExtraKWArg(VALUE maybeHash) {
    if (maybeHash == RUBY_Qundef) {
        return RUBY_Qundef;
    }
    if (RHASH_EMPTY_P(maybeHash)) {
        return RUBY_Qundef;
    }

    sorbet_raiseExtraKeywords(maybeHash);
}

SORBET_INLINE
VALUE sorbet_readKWRestArgs(VALUE maybeHash) {
    // This is similar to what the Ruby VM does:
    // https://github.com/ruby/ruby/blob/37c2cd3fa47c709570e22ec4dac723ca211f423a/vm_args.c#L483-L487
    if (maybeHash == RUBY_Qundef) {
        return rb_hash_new();
    }
    return rb_hash_dup(maybeHash);
}

SORBET_INLINE
VALUE sorbet_readRestArgs(int maxPositionalArgCount, int actualArgCount, VALUE *argArray) {
    if (maxPositionalArgCount >= actualArgCount) {
        return rb_ary_new();
    }
    return rb_ary_new_from_values(actualArgCount - maxPositionalArgCount, argArray + maxPositionalArgCount);
}

SORBET_INLINE
const VALUE **sorbet_getPc(rb_control_frame_t *cfp) {
    return &cfp->pc;
}

SORBET_INLINE
const VALUE *sorbet_getIseqEncoded(rb_control_frame_t *cfp) {
    return cfp->iseq->body->iseq_encoded;
}

SORBET_INLINE
void sorbet_setLineNumber(int offset, VALUE *iseq_encoded, VALUE **storeLocation) {
    // use pos+1 because PC should point at the next instruction
    (*storeLocation) = iseq_encoded + offset + 1;
}

SORBET_INLINE
VALUE sorbet_callBlock(VALUE array) {
    // TODO: one day we should use rb_yield_values, as it saves an allocation, but
    // for now, do the easy thing
    return rb_yield_splat(array);
}

// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm_insnhelper.h#L123
#define GET_PREV_EP(ep) ((VALUE *)((ep)[VM_ENV_DATA_INDEX_SPECVAL] & ~0x03))

// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm_insnhelper.c#L2919-L2928
static const VALUE *vm_get_ep(const VALUE *const reg_ep, rb_num_t lv) {
    rb_num_t i;
    const VALUE *ep = reg_ep;
    for (i = 0; i < lv; i++) {
        ep = GET_PREV_EP(ep);
    }
    return ep;
}

SORBET_INLINE
static int computeLocalIndex(long baseOffset, long index) {
    // Local offset calculation needs to take into account the fixed values that
    // are present on the stack:
    // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/compile.c#L1509
    return baseOffset + index + VM_ENV_DATA_SIZE;
}

// Read a value from the locals from this stack frame.
//
// * localsOffset - Offset into the locals (for static-init)
// * index - local var index
// * level - the number of blocks that need to be crossed to reach the
//           outer-most stack frame.
SORBET_INLINE
VALUE sorbet_readLocal(long localsOffset, long index, long level) {
    int offset = computeLocalIndex(localsOffset, index);
    return *(vm_get_ep(GET_EC()->cfp->ep, level) - offset);
}

// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm_insnhelper.c#L361-L371
SORBET_INLINE
static inline void vm_env_write(const VALUE *ep, int index, VALUE v) {
    VALUE flags = ep[VM_ENV_DATA_INDEX_FLAGS];
    if (LIKELY((flags & VM_ENV_FLAG_WB_REQUIRED) == 0)) {
        VM_STACK_ENV_WRITE(ep, index, v);
    } else {
        sorbet_vm_env_write_slowpath(ep, index, v);
    }
}

// Write a value into the locals from this stack frame.
//
// * localsOffset - Offset into the locals (for static-init)
// * index - local var index
// * level - the number of blocks that need to be crossed to reach the
//           outer-most stack frame.
// * value - the value to write
SORBET_INLINE
void sorbet_writeLocal(long localsOffset, long index, long level, VALUE value) {
    int offset = computeLocalIndex(localsOffset, index);
    vm_env_write(vm_get_ep(GET_EC()->cfp->ep, level), -offset, value);
}

SORBET_INLINE
VALUE sorbet_callFuncProcWithCache(VALUE recv, ID func, int argc,
                                   SORBET_ATTRIBUTE(noescape) const VALUE *const restrict argv, int kw_splat,
                                   VALUE proc, struct FunctionInlineCache *cache) {
    if (!NIL_P(proc)) {
        // this is an inlined version of vm_passed_block_handler_set(GET_EC(), proc);
        vm_block_handler_verify(proc);
        GET_EC()->passed_block_handler = proc;
    }

    return sorbet_callFuncWithCache(recv, func, argc, argv, kw_splat, cache);
}

// This function doesn't benefit from inlining, as it's always indirectly used through rb_iterate. In the future, if we
// end up with an inlined version of rb_iterate, it would be good to inline this.
static VALUE sorbet_iterMethod(VALUE obj) {
    struct sorbet_iterMethodArg *arg = (struct sorbet_iterMethodArg *)obj;
    return sorbet_callFuncWithCache(arg->recv, arg->func, arg->argc, arg->argv, arg->kw_splat, arg->cache);
}

SORBET_INLINE
VALUE sorbet_callFuncBlockWithCache(VALUE recv, ID func, int argc,
                                    SORBET_ATTRIBUTE(noescape) const VALUE *const restrict argv, int kw_splat,
                                    BlockFFIType blockImpl, VALUE closure, struct FunctionInlineCache *cache) {
    struct sorbet_iterMethodArg arg;
    arg.recv = recv;
    arg.func = func;
    arg.argc = argc;
    arg.argv = argv;
    arg.kw_splat = kw_splat;
    arg.cache = cache;

    return rb_iterate(sorbet_iterMethod, (VALUE)&arg, blockImpl, closure);
}

SORBET_INLINE
const rb_control_frame_t *sorbet_setRubyStackFrame(_Bool isClassOrModuleStaticInit, int iseq_type,
                                                   unsigned char *iseqchar) {
    const rb_iseq_t *iseq = (const rb_iseq_t *)iseqchar;
    rb_execution_context_t *ec = GET_EC();
    rb_control_frame_t *cfp = ec->cfp;

    // Depending on what kind of iseq we're switching to, we need to push a frame on the ruby stack.
    if (iseq_type == ISEQ_TYPE_RESCUE || iseq_type == ISEQ_TYPE_ENSURE) {
        sorbet_setExceptionStackFrame(ec, cfp, iseq);
    } else if (!isClassOrModuleStaticInit) {
        cfp->iseq = iseq;
        VM_ENV_FLAGS_UNSET(cfp->ep, VM_FRAME_FLAG_CFRAME);

        // For methods, allocate their locals on the ruby stack. This mirrors the implementation in vm_push_frame:
        // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm_insnhelper.c#L214-L248
        if (iseq_type == ISEQ_TYPE_METHOD) {
            sorbet_setMethodStackFrame(ec, cfp, iseq);
        }
    }

    return cfp;
}

SORBET_INLINE
VALUE sorbet_callSuper(int argc, SORBET_ATTRIBUTE(noescape) const VALUE *const restrict argv, int kw_splat) {
    // Mostly an implementation of return rb_call_super(argc, argv);
    rb_execution_context_t *ec = GET_EC();
    VALUE recv = ec->cfp->self;
    VALUE klass;
    ID id;
    rb_control_frame_t *cfp = ec->cfp;
    const rb_callable_method_entry_t *me = rb_vm_frame_method_entry(cfp);

    klass = RCLASS_ORIGIN(me->defined_class);
    klass = RCLASS_SUPER(klass);
    id = me->def->original_id;
    me = rb_callable_method_entry(klass, id);

    if (!me) {
        // TODO do something here
        // return rb_method_missing(recv, id, argc, argv, MISSING_SUPER);
        rb_raise(rb_eRuntimeError, "unimplemented super with a missing method");
        return Qnil;
    } else {
        return rb_vm_call(ec, recv, id, argc, argv, me);
    }
}

// ****
// ****                       Exceptions
// ****

SORBET_INLINE
VALUE sorbet_getTRetry() {
    static const char retry[] = "T::Private::Retry::RETRY";
    return sorbet_getConstant(retry, sizeof(retry));
}

extern void rb_set_errinfo(VALUE);

struct ExceptionClosure {
    ExceptionFFIType body;
    VALUE **pc;
    VALUE *iseq_encoded;
    VALUE methodClosure;
    VALUE *returnValue;
};

static VALUE sorbet_applyExceptionClosure(VALUE arg) {
    struct ExceptionClosure *closure = (struct ExceptionClosure *)arg;
    VALUE res = closure->body(closure->pc, closure->iseq_encoded, closure->methodClosure);
    if (res != sorbet_rubyUndef()) {
        *closure->returnValue = res;
    }
    return sorbet_rubyUndef();
}

static VALUE sorbet_rescueStoreException(VALUE exceptionValuePtr) {
    VALUE *exceptionValue = (VALUE *)exceptionValuePtr;

    // fetch the last exception, and store it in the dest var passed in;
    *exceptionValue = rb_errinfo();

    return sorbet_rubyUndef();
}

// Run a function with a closure, and populate an exceptionValue pointer if an exception is raised.
VALUE sorbet_try(ExceptionFFIType body, VALUE **pc, VALUE *iseq_encoded, VALUE methodClosure, VALUE exceptionContext,
                 VALUE *exceptionValue) {
    VALUE returnValue = sorbet_rubyUndef();

    struct ExceptionClosure closure;
    closure.body = body;
    closure.pc = pc;
    closure.iseq_encoded = iseq_encoded;
    closure.methodClosure = methodClosure;
    closure.returnValue = &returnValue;

    *exceptionValue = RUBY_Qnil;

    // Restore the exception context. When running the body of a begin/end the
    // value of exceptionContext is nil, indicating that no exception is being
    // handled by this function. However, when the rescue function is being run,
    // the exception value will be non-nil, ensuring that the exception state
    // is restored in the context of the rescue function.
    if (exceptionContext != RUBY_Qnil) {
        rb_set_errinfo(exceptionContext);
    }

    rb_rescue2(sorbet_applyExceptionClosure, (VALUE)(&closure), sorbet_rescueStoreException, (VALUE)exceptionValue,
               rb_eException, 0);

    return returnValue;
}

__attribute__((__noreturn__)) VALUE sorbet_block_break(VALUE recv, ID fun, int argc, const VALUE *const restrict argv,
                                                       BlockFFIType blk, VALUE closure) {
    rb_iter_break_value(argv[0]);
}

// Raise the exception value, unless it's nil.
SORBET_INLINE
void sorbet_raiseIfNotNil(VALUE exception) {
    if (exception == RUBY_Qnil) {
        return;
    }

    rb_exc_raise(exception);
}

// Ruby passes the RTLD_LAZY flag to the dlopen(3) call (which is supported by both macOS and Linux).
// That flag says, "Only resolve symbols as the code that references them is executed. If the symbol
// is never referenced, then it is never resolved."
//
// Thus, by putting our version check first before any other code in the C extension runs, and backing
// up the symbols our version check relies on with weak symbols, we can guarantee that the user never
// sees a symbol resolution error from loading a shared object when they shouldn't have.
SORBET_INLINE
void sorbet_ensureSorbetRuby(int compile_time_is_release_build, char *compile_time_build_scm_revision) {
    if (!compile_time_is_release_build) {
        // Skipping version check: This shared object was compiled by a non-release version of SorbetLLVM
        return;
    }

    const int runtime_is_release_build = sorbet_getIsReleaseBuild();
    if (!runtime_is_release_build) {
        // Skipping version check: sorbet_ruby is a non-release version
        return;
    }

    const char *runtime_build_scm_revision = sorbet_getBuildSCMRevision();
    if (strcmp(compile_time_build_scm_revision, runtime_build_scm_revision) != 0) {
        rb_raise(rb_eRuntimeError,
                 "SorbetLLVM runtime version mismatch: sorbet_ruby compiled with %s but shared object compiled with %s",
                 runtime_build_scm_revision, compile_time_build_scm_revision);
    }
}

// ****
// ****                       sorbet_ruby version information fallback
// ****

// A strong version of these functions will be linked into libruby.so when Ruby is built as sorbet_ruby.
// When our compiled C extensions are loaded by sorbet_ruby, calls will resolve to the symbol inside libruby.so.
// When our compiled C extensions are loaded by a system Ruby or an rbenv-built Ruby, these weak symbols act as
// a fallback so that we can gracefully exit (Ruby exception) when not run under sorbet_ruby instead of
// ungracefully exit (dynamic symbol resolution error + corrupt Ruby VM).
const char *sorbet_getBuildSCMRevision() __attribute__((weak)) {
    rb_raise(rb_eRuntimeError,
             "sorbet_getBuildSCMRevision: Shared objects compiled by sorbet_llvm must be run by sorbet_ruby.");
}

const int sorbet_getIsReleaseBuild() __attribute__((weak)) {
    rb_raise(rb_eRuntimeError,
             "sorbet_getIsReleaseBuild: Shared objects compiled by sorbet_llvm must be run by sorbet_ruby.");
}

// These forward declarations don't actually exist except in the LLVM IR we generate for each C extension,
// so this function fails to link when compiling the payload into libruby.so.
//
// We don't actually need this function to be present in that shared object, so we can omit it.

VALUE sorbet_i_getRubyClass(const char *const className, long classNameLen) __attribute__((const));
VALUE sorbet_i_getRubyConstant(const char *const className, long classNameLen) __attribute__((const));

VALUE __sorbet_only_exists_to_keep_functions_alive__() __attribute__((optnone)) {
    // this function will be nuked but it exists to keep forward definitions alive for clang
    return (long)&sorbet_i_getRubyClass + (long)&sorbet_i_getRubyConstant + (long)&sorbet_getConstantEpoch +
           (long)&sorbet_getConstant + (long)&rb_id2sym;
}
