//
// payload.c
//
// Most of the APIs we're using here are normal Ruby C extension APIs.
// Suggested reading:
//
// - <https://silverhammermba.github.io/emberb/c/>
// - <http://clalance.blogspot.com/2011/01/writing-ruby-extensions-in-c-part-9.html>
//
// You'll also find a lot of useful information from grepping the Ruby source code.
//

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

// SORBET_ATTRIBUTE is for specifying attributes that are used for their effects
// on llvm optimization. Usually this is for attributes like `alwaysinline`,
// which will cause build failures in libruby as these functions will never
// appear to be called.
#ifdef SORBET_LLVM_PAYLOAD
#define SORBET_ATTRIBUTE(...) __attribute__((__VA_ARGS__))
#else
#define SORBET_ATTRIBUTE(...) /* empty */
#endif

#define SORBET_INLINE SORBET_ATTRIBUTE(always_inline)

// Paul's and Dmitry's laptops have different attributes for this function in system libraries.
void abort(void) __attribute__((__cold__)) __attribute__((__noreturn__));

// Common definitions

typedef VALUE (*BlockFFIType)(VALUE firstYieldedArg, VALUE closure, int argCount, VALUE *args, VALUE blockArg);

typedef VALUE (*ExceptionFFIType)(VALUE **pc, VALUE *iseq_encoded, VALUE closure);

// ****
// ****                       Internal Helper Functions
// ****

const char *sorbet_dbg_pi(ID id) {
    return rb_id2name(id);
}

const char *sorbet_dbg_p(VALUE obj) {
    char *ret = RSTRING_PTR(rb_sprintf("%" PRIsVALUE, obj));
    return ret;
}

void sorbet_stopInDebugger() {
    __asm__("int $3");
}

// ****
// ****                       sorbet_ruby version information fallback
// ****
#ifdef SORBET_LLVM_PAYLOAD

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

#endif

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

// use this undefined value when you have a variable that should _never_ escape to ruby.
SORBET_INLINE
VALUE sorbet_rubyUndef() {
    return RUBY_Qundef;
}

SORBET_INLINE
VALUE sorbet_rubyNil() {
    return RUBY_Qnil;
}

SORBET_INLINE
SORBET_ATTRIBUTE(pure)
VALUE sorbet_rubyTopSelf() {
    return GET_VM()->top_self;
}

SORBET_INLINE
SORBET_ATTRIBUTE(pure)
VALUE sorbet_rubyException() {
    return rb_eException;
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
// ****                       Integer
// ****

SORBET_INLINE
VALUE sorbet_Integer_plus_Integer(VALUE a, VALUE b) {
    return sorbet_longToRubyValue(sorbet_rubyValueToLong(a) + sorbet_rubyValueToLong(b));
}

SORBET_INLINE
VALUE sorbet_Integer_minus_Integer(VALUE a, VALUE b) {
    return sorbet_longToRubyValue(sorbet_rubyValueToLong(a) - sorbet_rubyValueToLong(b));
}

SORBET_INLINE
VALUE sorbet_Integer_less_Integer(VALUE a, VALUE b) {
    return (sorbet_rubyValueToLong(a) < sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

SORBET_INLINE
VALUE sorbet_Integer_greater_Integer(VALUE a, VALUE b) {
    return (sorbet_rubyValueToLong(a) > sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

SORBET_INLINE
VALUE sorbet_Integer_greatereq_Integer(VALUE a, VALUE b) {
    return (sorbet_rubyValueToLong(a) >= sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

SORBET_INLINE
VALUE sorbet_Integer_lesseq_Integer(VALUE a, VALUE b) {
    return (sorbet_rubyValueToLong(a) <= sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

SORBET_INLINE
VALUE sorbet_Integer_eq_Integer(VALUE a, VALUE b) {
    return (sorbet_rubyValueToLong(a) == sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

SORBET_INLINE
VALUE sorbet_Integer_neq_Integer(VALUE a, VALUE b) {
    return (sorbet_rubyValueToLong(a) != sorbet_rubyValueToLong(b)) ? RUBY_Qtrue : RUBY_Qfalse;
}

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
void sorbet_hashStore(VALUE hash, VALUE key, VALUE value) {
    rb_hash_aset(hash, key, value);
}

SORBET_INLINE
VALUE sorbet_hashGet(VALUE hash, VALUE key) {
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
// ****                       Tests
// ****

SORBET_INLINE
_Bool sorbet_testIsTruthy(VALUE value) {
    return RB_TEST(value);
}

SORBET_INLINE
_Bool sorbet_testIsTrue(VALUE value) {
    return value == RUBY_Qtrue;
}

SORBET_INLINE
_Bool sorbet_testIsFalse(VALUE value) {
    return value == RUBY_Qfalse;
}

SORBET_INLINE
_Bool sorbet_testIsNil(VALUE value) {
    return value == RUBY_Qnil;
}

SORBET_INLINE
_Bool sorbet_testIsUndef(VALUE value) {
    return value == RUBY_Qundef;
}

SORBET_INLINE
_Bool sorbet_testIsSymbol(VALUE value) {
    return RB_SYMBOL_P(value);
}

SORBET_INLINE
_Bool sorbet_testIsFloat(VALUE value) {
    return RB_FLOAT_TYPE_P(value);
}

SORBET_INLINE
_Bool sorbet_testIsHash(VALUE value) {
    return TYPE(value) == RUBY_T_HASH;
}

SORBET_INLINE
_Bool sorbet_testIsArray(VALUE value) {
    return TYPE(value) == RUBY_T_ARRAY;
}

SORBET_INLINE
_Bool sorbet_testIsString(VALUE value) {
    return TYPE(value) == RUBY_T_STRING;
}

// https://ruby-doc.org/core-2.6.3/Object.html#method-i-eql-3F
SORBET_INLINE
_Bool sorbet_testObjectEqual_p(VALUE obj1, VALUE obj2) {
    return obj1 == obj2;
}

// ****
// ****                       Variables
// ****

SORBET_INLINE
VALUE sorbet_instanceVariableGet(VALUE receiver, ID name) {
    return rb_ivar_get(receiver, name);
}

SORBET_INLINE
VALUE sorbet_instanceVariableSet(VALUE receiver, ID name, VALUE newValue) {
    return rb_ivar_set(receiver, name, newValue);
}

VALUE sorbet_globalVariableGet(ID name) {
    return rb_gvar_get(rb_global_entry(name));
}

void sorbet_globalVariableSet(ID name, VALUE newValue) {
    rb_gvar_set(rb_global_entry(name), newValue);
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
// ****                       Constants, Classes and Modules
// ****

VALUE sorbet_rb_cObject() {
    return rb_cObject;
}

SORBET_INLINE
void sorbet_defineTopLevelConstant(const char *name, VALUE value) {
    rb_define_global_const(name, value);
}

SORBET_INLINE
void sorbet_defineNestedCosntant(VALUE owner, const char *name, VALUE value) {
    rb_define_const(owner, name, value);
}

VALUE sorbet_getMethodBlockAsProc() {
    if (rb_block_given_p()) {
        return rb_block_proc();
    }
    return Qnil;
}

// Trying to be a copy of rb_mod_const_get
SORBET_ATTRIBUTE(noinline)
VALUE sorbet_getConstantAt(VALUE mod, ID id) {
    VALUE name;
    rb_encoding *enc;
    const char *pbeg, *p, *path, *pend;
    int recur = 1;
    int DISABLED_CODE = 0;

    name = rb_id2str(id);
    enc = rb_enc_get(name);
    path = rb_id2name(id);

    pbeg = p = path;
    pend = path + strlen(path);

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
            VALUE idConst_missing = rb_intern("const_missing");
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

SORBET_ATTRIBUTE(noinline)
VALUE sorbet_getConstant(const char *path, long pathLen) {
    VALUE mod = sorbet_rb_cObject();
    ID id = rb_intern2(path, pathLen);
    return sorbet_getConstantAt(mod, id);
}

SORBET_ATTRIBUTE(noinline)
void sorbet_setConstant(VALUE mod, const char *name, long nameLen, VALUE value) {
    ID id = rb_intern2(name, nameLen);
    return rb_const_set(mod, id, value);
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
// ****                       Calls
// ****

SORBET_INLINE
VALUE sorbet_callBlock(VALUE array) {
    // TODO: one day we should use rb_yield_values, as it saves an allocation, but
    // for now, do the easy thing
    return rb_yield_splat(array);
}

SORBET_INLINE
VALUE sorbet_callSuper(int argc, SORBET_ATTRIBUTE(noescape) const VALUE *const restrict argv) {
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

__attribute__((__cold__, __noreturn__)) void sorbet_cast_failure(VALUE value, char *castMethod, char *type) {
    // TODO: cargo cult more of
    // https://github.com/sorbet/sorbet/blob/b045fb1ba12756c3760fe516dc315580d93f3621/gems/sorbet-runtime/lib/types/types/base.rb#L105
    //
    // e.g. we need to teach the `got` part to do `T.class_of`
    rb_raise(rb_eTypeError, "%s: Expected type %s, got type %s with value %" PRIsVALUE, castMethod, type,
             rb_obj_classname(value), value);
}

__attribute__((__noreturn__)) void sorbet_raiseArity(int argc, int min, int max) {
    rb_exc_raise(sorbet_rb_arity_error_new(argc, min, max));
}

__attribute__((__noreturn__)) void sorbet_raiseExtraKeywords(VALUE hash) {
    VALUE err_mess = rb_sprintf("unknown keywords: %" PRIsVALUE, rb_hash_keys(hash));
    rb_exc_raise(rb_exc_new3(rb_eArgError, err_mess));
}

__attribute__((__cold__)) VALUE sorbet_t_absurd(VALUE val) {
    VALUE t = rb_const_get(rb_cObject, rb_intern("T"));
    return rb_funcall(t, rb_intern("absurd"), 1, val);
}

void sorbet_checkStack() {
    // This is actually pretty slow. We should probably use guard pages instead.
    ruby_stack_check();
}

// ****
// **** Optimized versions of callFunc.
// **** Should use the same calling concention.
// **** Call it ending with `_no_type_guard` if implementation has a backed in slowpath
// ****
// ****

// ****
// ****                       Closures
// ****

// this specifies to use ruby default free for freeing(which is just xfree). Thus objects should be allocated with
// xmalloc

struct sorbet_Closure {
    const int size;
    VALUE closureData[]; // this is a rarely known feature of C99 https://en.wikipedia.org/wiki/Flexible_array_member
};

struct sorbet_Closure *sorbet_Closure_alloc(int elemCount) {
    struct sorbet_Closure *ret =
        (struct sorbet_Closure *)xmalloc(sizeof(struct sorbet_Closure) + sizeof(VALUE) * elemCount);
    *(int *)&ret->size = elemCount; // this is how you assign a const field after malloc
    return ret;
}

void sorbet_Closure_mark(void *closurePtr) {
    // this might be possible to make more efficient using rb_mark_tbl
    struct sorbet_Closure *ptr = (struct sorbet_Closure *)closurePtr;
    rb_gc_mark_values(ptr->size, &ptr->closureData[0]);
}

SORBET_ATTRIBUTE(const)
size_t sorbet_Closure_size(const void *closurePtr) {
    // this might be possible to make more efficient using rb_mark_tbl
    struct sorbet_Closure *ptr = (struct sorbet_Closure *)closurePtr;
    return sizeof(struct sorbet_Closure) + ptr->size * sizeof(VALUE);
}

#ifdef SORBET_LLVM_PAYLOAD

// This version is used when generating llvm from the payload, so that we are always using the function for fetching the
// closureInfo pointer that resides in the ruby VM.
extern const rb_data_type_t *sorbet_getClosureInfo();

#else

// This code is linked directly into sorbet_ruby.

static const rb_data_type_t closureInfo = {
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

const rb_data_type_t *sorbet_getClosureInfo() {
    return &closureInfo;
}
#endif

VALUE sorbet_allocClosureAsValue(int elemCount) {
    struct sorbet_Closure *ptr = sorbet_Closure_alloc(elemCount);
    const rb_data_type_t *info = sorbet_getClosureInfo();
    return TypedData_Wrap_Struct(rb_cData, info, ptr);
}

VALUE *sorbet_getClosureElem(VALUE closure, int elemId) {
    struct sorbet_Closure *ptr = (struct sorbet_Closure *)RTYPEDDATA_DATA(closure);
    return &(ptr->closureData[elemId]);
}

// ****
// ****                       Control Frames
// ****

/* From inseq.h */
struct rb_compile_option_struct {
    unsigned int inline_const_cache : 1;
    unsigned int peephole_optimization : 1;
    unsigned int tailcall_optimization : 1;
    unsigned int specialized_instruction : 1;
    unsigned int operands_unification : 1;
    unsigned int instructions_unification : 1;
    unsigned int stack_caching : 1;
    unsigned int frozen_string_literal : 1;
    unsigned int debug_frozen_string_literal : 1;
    unsigned int coverage_enabled : 1;
    int debug_level;
};

struct iseq_insn_info_entry {
    int line_no;
    rb_event_flag_t events;
};

void rb_iseq_insns_info_encode_positions(const rb_iseq_t *iseq);
/* End from inseq.h */

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

// NOTE: parent is the immediate parent frame, so for the rescue clause of a
// top-level method the parent would be the method iseq, but for a rescue clause
// nested within a rescue clause, it would be the outer rescue iseq.
//
// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/compile.c#L5669-L5671
void *sorbet_allocateRubyStackFrame(VALUE funcName, ID func, VALUE filename, VALUE realpath, unsigned char *parent,
                                    int iseqType, int startline, int endline, ID *locals, int numLocals) {
    // DO NOT ALLOCATE RUBY LEVEL OBJECTS HERE. All objects that are passed to
    // this function should be retained (for GC purposes) by something else.

    // ... but actually this line allocates and will not be retained by anyone else,
    // so we pin this object right here. TODO: This leaks memory
    rb_iseq_t *iseq = rb_iseq_new(0, funcName, filename, realpath, (rb_iseq_t *)parent, iseqType);
    rb_gc_register_mark_object((VALUE)iseq);

    // This is the table that tells us the hash entry for instruction types
    const void *const *table = rb_vm_get_insns_address_table();
    VALUE nop = (VALUE)table[YARVINSN_nop];

    // Even if start and end are on the same line, we still want one insns_info made
    int insn_num = endline - startline + 1;
    struct iseq_insn_info_entry *insns_info = ALLOC_N(struct iseq_insn_info_entry, insn_num);
    unsigned int *positions = ALLOC_N(unsigned int, insn_num);
    VALUE *iseq_encoded = ALLOC_N(VALUE, insn_num);
    for (int i = 0; i < insn_num; i++) {
        int lineno = i + startline;
        positions[i] = i;
        insns_info[i].line_no = lineno;

        // we fill iseq_encoded with NOP instructions; it only exists because it
        // has to match the length of insns_info.
        iseq_encoded[i] = nop;
    }
    iseq->body->insns_info.body = insns_info;
    iseq->body->insns_info.positions = positions;
    // One iseq per line
    iseq->body->iseq_size = insn_num;
    iseq->body->insns_info.size = insn_num;
    rb_iseq_insns_info_encode_positions(iseq);

    // One NOP per line, to match insns_info
    iseq->body->iseq_encoded = iseq_encoded;

    // if this is a rescue frame, we need to set up some local storage for
    // exception values ($!).
    if (iseqType == ISEQ_TYPE_RESCUE || iseqType == ISEQ_TYPE_ENSURE) {
        // this is an inlined version of `iseq_set_exception_local_table` from
        // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/compile.c#L1390-L1404
        ID id_dollar_bang;
        ID *ids = (ID *)ALLOC_N(ID, 1);

        CONST_ID(id_dollar_bang, "#$!");
        iseq->body->local_table_size = 1;
        ids[0] = id_dollar_bang;
        iseq->body->local_table = ids;
    }

    if (iseqType == ISEQ_TYPE_METHOD && numLocals > 0) {
        // this is a simplified version of `iseq_set_local_table` from
        // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/compile.c#L1767-L1789

        // allocate space for the names of the locals
        ID *ids = (ID *)ALLOC_N(ID, numLocals);

        memcpy(ids, locals, numLocals * sizeof(ID));

        iseq->body->local_table = ids;
        iseq->body->local_table_size = numLocals;
    }

    // Cast it to something easy since teaching LLVM about structs is a huge PITA
    return (void *)iseq;
}

const VALUE sorbet_readRealpath() {
    VALUE realpath = rb_gv_get("$__sorbet_ruby_realpath");
    if (!RB_TYPE_P(realpath, T_STRING)) {
        rb_raise(rb_eRuntimeError, "Invalid '$__sorbet_ruby_realpath' when loading compiled module");
    }

    rb_gv_set("$__sorbet_ruby_realpath", sorbet_rubyNil());
    return realpath;
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

// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm_insnhelper.c#L349-L359
SORBET_ATTRIBUTE(noinline)
static void vm_env_write_slowpath(const VALUE *ep, int index, VALUE v) {
    /* remember env value forcely */
    rb_gc_writebarrier_remember(VM_ENV_ENVVAL(ep));
    VM_FORCE_WRITE(&ep[index], v);
    VM_ENV_FLAGS_UNSET(ep, VM_ENV_FLAG_WB_REQUIRED);
}

// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm_insnhelper.c#L361-L371
SORBET_INLINE
static inline void vm_env_write(const VALUE *ep, int index, VALUE v) {
    VALUE flags = ep[VM_ENV_DATA_INDEX_FLAGS];
    if (LIKELY((flags & VM_ENV_FLAG_WB_REQUIRED) == 0)) {
        VM_STACK_ENV_WRITE(ep, index, v);
    } else {
        vm_env_write_slowpath(ep, index, v);
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

// This is an inlined version of c_stack_overflow(GET_EC(), TRUE).
//
// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm_insnhelper.c#L34-L48
SORBET_ATTRIBUTE(__noreturn__, always_inline)
static void sorbet_stackoverflow() {
    // visible, but not exported through a header
    extern VALUE rb_ec_backtrace_object(const rb_execution_context_t *);

    rb_execution_context_t *ec = GET_EC();

    VALUE mesg = rb_ec_vm_ptr(ec)->special_exceptions[ruby_error_sysstack];
    // The original line here is:
    // > ec->raised_flag = RAISED_STACKOVERFLOW;
    // but RAISED_STACKOVERFLOW (value 2) is defined in eval_intern.h which
    // causes compile errors when we include it.
    ec->raised_flag = 2;

    VALUE at = rb_ec_backtrace_object(ec);
    mesg = ruby_vm_special_exception_copy(mesg);
    rb_ivar_set(mesg, idBt, at);
    rb_ivar_set(mesg, idBt_locations, at);

    ec->errinfo = mesg;

    // This is an inlined version of EC_JUMP_TAG(ec, TAG_RAISE). EC_JUMP_TAG is
    // defined in eval_intern.h, which causes compile errors when we include it.
    ec->tag->state = TAG_RAISE;
    RUBY_LONGJMP(ec->tag->buf, 1);
}

/* Defined in vm_insnhelper.c, but not exported via any header */
extern rb_control_frame_t *rb_vm_push_frame(rb_execution_context_t *sec, const rb_iseq_t *iseq, VALUE type, VALUE self,
                                            VALUE specval, VALUE cref_or_me, const VALUE *pc, VALUE *sp, int local_size,
                                            int stack_max);

// NOTE: this is marked noinline so that there's only ever one copy that lives in the ruby runtime, cutting down on the
// size of generated artifacts. If we decide that speed is more important, this could be marked alwaysinline to avoid
// the function call.
SORBET_ATTRIBUTE(noinline)
void sorbet_setExceptionStackFrame(rb_execution_context_t *ec, rb_control_frame_t *cfp, const rb_iseq_t *iseq) {
    // Self is the same in exception-handlers
    VALUE self = cfp->self;

    // there is only ever one local for rescue/ensure frames, this mirrors the implementation in
    // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm.c#L2085-L2101
    VALUE *sp = cfp->sp + 1;
    int num_locals = iseq->body->local_table_size - 1;

    VALUE blockHandler = VM_GUARDED_PREV_EP(cfp->ep);
    VALUE me = 0;

    // write the exception value on the stack (as an argument to the rescue frame)
    cfp->sp[0] = rb_errinfo();

    // NOTE: there's no explicit check for stack overflow, because `rb_vm_push_frame` will do that check
    cfp = rb_vm_push_frame(ec, iseq, VM_FRAME_MAGIC_RESCUE, self, blockHandler, me, iseq->body->iseq_encoded, sp,
                           num_locals, iseq->body->stack_max);
}

// NOTE: this is marked noinline so that there's only ever one copy that lives in the ruby runtime, cutting down on the
// size of generated artifacts. If we decide that speed is more important, this could be marked alwaysinline to avoid
// the function call.
SORBET_ATTRIBUTE(noinline)
void sorbet_setMethodStackFrame(rb_execution_context_t *ec, rb_control_frame_t *cfp, const rb_iseq_t *iseq) {
    // make sure that we have enough space to allocate locals
    int local_size = iseq->body->local_table_size;
    int stack_max = iseq->body->stack_max;

    WHEN_VM_STACK_OVERFLOWED(cfp, cfp->sp, local_size + stack_max) sorbet_stackoverflow();

    // save the current state of the stack
    VALUE cref_or_me = cfp->ep[VM_ENV_DATA_INDEX_ME_CREF]; // -2
    VALUE prev_ep = cfp->ep[VM_ENV_DATA_INDEX_SPECVAL];    // -1
    VALUE type = cfp->ep[VM_ENV_DATA_INDEX_FLAGS];         // -0

    // C frames push no locals, so the address we want to treat as the top of the stack lies at -2.
    // NOTE: we're explicitly discarding the const qualifier from ep, because we need to write to the stack.
    VALUE *sp = (VALUE *)&cfp->ep[-2];

    // setup locals
    for (int i = 0; i < local_size; ++i) {
        *sp++ = RUBY_Qnil;
    }

    // restore the previous state of the stack
    *sp++ = cref_or_me;
    *sp++ = prev_ep;
    *sp = type;

    cfp->ep = cfp->bp = sp;
    cfp->sp = sp + 1;
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

extern void rb_vm_pop_frame(rb_execution_context_t *ec);

void sorbet_popRubyStack() {
    rb_vm_pop_frame(GET_EC());
}

const VALUE **sorbet_getPc(rb_control_frame_t *cfp) {
    return &cfp->pc;
}

const VALUE *sorbet_getIseqEncoded(rb_control_frame_t *cfp) {
    return cfp->iseq->body->iseq_encoded;
}

void sorbet_setLineNumber(int offset, VALUE *iseq_encoded, VALUE **storeLocation) {
    // use pos+1 because PC should point at the next instruction
    (*storeLocation) = iseq_encoded + offset + 1;
}

VALUE sorbet_getKWArg(VALUE maybeHash, VALUE key) {
    if (maybeHash == RUBY_Qundef) {
        return RUBY_Qundef;
    }

    // TODO: ruby seems to do something smarter here:
    //  https://github.com/ruby/ruby/blob/5aa0e6bee916f454ecf886252e1b025d824f7bd8/class.c#L1901
    //
    return rb_hash_delete_entry(maybeHash, key);
}

VALUE sorbet_assertNoExtraKWArg(VALUE maybeHash) {
    if (maybeHash == RUBY_Qundef) {
        return RUBY_Qundef;
    }
    if (RHASH_EMPTY_P(maybeHash)) {
        return RUBY_Qundef;
    }

    sorbet_raiseExtraKeywords(maybeHash);
}

VALUE sorbet_readKWRestArgs(VALUE maybeHash) {
    // This is similar to what the Ruby VM does:
    // https://github.com/ruby/ruby/blob/37c2cd3fa47c709570e22ec4dac723ca211f423a/vm_args.c#L483-L487
    if (maybeHash == RUBY_Qundef) {
        return rb_hash_new();
    }
    return rb_hash_dup(maybeHash);
}

VALUE sorbet_readRestArgs(int maxPositionalArgCount, int actualArgCount, VALUE *argArray) {
    if (maxPositionalArgCount >= actualArgCount) {
        return rb_ary_new();
    }
    return rb_ary_new_from_values(actualArgCount - maxPositionalArgCount, argArray + maxPositionalArgCount);
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

/*
_Bool sorbet_isa_Method(VALUE obj) __attribute__((const))  {
    return rb_obj_is_method(obj) == Qtrue;
}
*/

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_Proc(VALUE obj) {
    return rb_obj_is_proc(obj) == Qtrue;
}

SORBET_ATTRIBUTE(const)
_Bool sorbet_isa_RootSingleton(VALUE obj) {
    return obj == sorbet_rubyTopSelf();
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

// ****
// ****                       Helpers for Intrinsics
// ****

void sorbet_ensure_arity(int argc, int expected) {
    if (argc != expected) {
        sorbet_raiseArity(argc, expected, expected);
    }
}

VALUE sorbet_boolToRuby(_Bool b) {
    if (b) {
        return RUBY_Qtrue;
    }
    return RUBY_Qfalse;
}

// ****
// ****                       Name Based Intrinsics
// ****

VALUE sorbet_buildHashIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                VALUE closure) {
    VALUE ret = rb_hash_new_with_size(argc / 2);
    if (argc != 0) {
        // We can use rb_hash_bulk_insert here because rb_hash_new was freshly allocated.
        // We have tried in the past to use rb_hash_bulk_insert after clearing an existing hash,
        // and things broke wonderfully, because Ruby Hash objects are either backed by a small (<8 element)
        // or large hash table implementation, and neither Hash#clear nor rb_hash_bulk_insert doesn't
        // change what kind of Hash object it is.
        rb_hash_bulk_insert(argc, argv, ret);
    }
    return ret;
}

VALUE sorbet_buildArrayIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                 VALUE closure) {
    if (argc == 0) {
        return rb_ary_new();
    }
    return rb_ary_new_from_values(argc, argv);
}

VALUE sorbet_buildRangeIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                 VALUE closure) {
    sorbet_ensure_arity(argc, 3);

    VALUE start = argv[0];
    VALUE end = argv[1];
    VALUE excludeEnd = argv[2];
    return rb_range_new(start, end, excludeEnd);
}

VALUE sorbet_splatIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                            VALUE closure) {
    sorbet_ensure_arity(argc, 3);
    VALUE arr = argv[0];
    long len = sorbet_rubyArrayLen(arr);
    int size = sorbet_rubyValueToLong(argv[1]) + sorbet_rubyValueToLong(argv[2]);
    int missing = size - len;
    if (missing > 0) {
        VALUE newArr = rb_ary_dup(arr);
        for (int i = 0; i < missing; i++) {
            sorbet_arrayPush(newArr, sorbet_rubyNil());
        }
        return newArr;
    }
    return arr;
}

// This doesn't do exactly the right thing because that is done by the parser in Ruby. Ruby will return the String
// "expression" if the RHS is an expression.
VALUE sorbet_definedIntinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                             VALUE closure) {
    if (argc == 0) {
        return sorbet_rubyNil();
    }
    VALUE klass = sorbet_rb_cObject();
    for (int i = 0; i < argc; i++) {
        VALUE str = argv[i];
        ID id = rb_intern(sorbet_rubyStringToCPtr(str));
        if (!rb_const_defined_at(klass, id)) {
            return sorbet_rubyNil();
        }
        klass = sorbet_getConstantAt(klass, id);
    }
    return rb_str_new2("constant");
}

// ****
// ****                       Symbol Intrinsics. See CallCMethod in SymbolIntrinsics.cc
// ****

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
    return sorbet_longToRubyValue(rb_array_len(recv));
}

VALUE sorbet_rb_array_square_br_slowpath(VALUE recv, ID fun, int argc, const VALUE *const restrict argv,
                                         BlockFFIType blk, VALUE closure) {
    VALUE ary = recv;
    if (argc == 2) {
        return rb_ary_aref2(ary, argv[0], argv[1]);
    }
    VALUE arg = argv[0];

    long beg, len;

    /* check if idx is Range */
    switch (rb_range_beg_len(arg, &beg, &len, RARRAY_LEN(ary), 0)) {
        case Qfalse:
            break;
        case Qnil:
            return Qnil;
        default:
            return rb_ary_subseq(ary, beg, len);
    }
    return rb_ary_entry(ary, NUM2LONG(arg));
}
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

VALUE sorbet_rb_array_empty(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                            VALUE closure) {
    rb_check_arity(argc, 0, 0);
    if (RARRAY_LEN(recv) == 0) {
        return Qtrue;
    }
    return Qfalse;
}

VALUE sorbet_enumerator_size_func_array_length(VALUE array, VALUE args, VALUE eobj) {
    return RARRAY_LEN(array);
}

VALUE sorbet_rb_hash_square_br(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                               VALUE closure) {
    rb_check_arity(argc, 1, 1);
    return rb_hash_aref(recv, argv[0]);
}

VALUE sorbet_rb_hash_square_br_eq(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                                  VALUE closure) {
    rb_check_arity(argc, 2, 2);
    return rb_hash_aset(recv, argv[0], argv[1]);
}

VALUE sorbet_rb_int_plus_slowpath(VALUE recv, VALUE y) {
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

VALUE sorbet_rb_int_minus_slowpath(VALUE recv, VALUE y) {
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

VALUE sorbet_rb_int_mul(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                        VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    return rb_int_mul(recv, argv[0]);
}

VALUE sorbet_rb_int_div(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                        VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    return rb_int_div(recv, argv[0]);
}

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
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_cmp(y, recv) == INT2FIX(-1) ? Qtrue : Qfalse;
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return rb_integer_float_cmp(recv, y) == INT2FIX(+1) ? Qtrue : Qfalse;
        }
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_gt(recv, y);
    }
    return rb_num_coerce_relop(recv, y, '>');
}

VALUE sorbet_rb_int_lt_slowpath(VALUE recv, VALUE y) {
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
        }
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_lt(recv, y);
    }
    return rb_num_coerce_relop(recv, y, '<');
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

VALUE sorbet_rb_int_ge(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                       VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    VALUE res = sorbet_rb_int_lt(recv, fun, argc, argv, blk, closure);
    return res == Qtrue ? Qfalse : Qtrue;
}

VALUE sorbet_rb_int_le(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                       VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    VALUE res = sorbet_rb_int_gt(recv, fun, argc, argv, blk, closure);
    return res == Qtrue ? Qfalse : Qtrue;
}

VALUE sorbet_rb_int_equal(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                          VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    return rb_int_equal(recv, argv[0]);
}

VALUE sorbet_rb_int_neq(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                        VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    return sorbet_boolToRuby(rb_int_equal(recv, argv[0]) == sorbet_rubyFalse());
}

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

extern VALUE rb_obj_as_string_result(VALUE, VALUE);

extern VALUE rb_str_concat_literals(int, const VALUE *const restrict);

VALUE sorbet_stringInterpolate(VALUE recv, ID fun, int argc, VALUE *argv, BlockFFIType blk, VALUE closure) {
    for (int i = 0; i < argc; ++i) {
        if (!RB_TYPE_P(argv[i], T_STRING)) {
            VALUE str = rb_funcall(argv[i], idTo_s, 0);
            argv[i] = rb_obj_as_string_result(str, argv[i]);
        }
    }

    return rb_str_concat_literals(argc, argv);
}

VALUE sorbet_selfNew(VALUE recv, ID fun, int argc, VALUE *argv, BlockFFIType blk, VALUE closure) {
    rb_check_arity(argc, 1, UNLIMITED_ARGUMENTS);
    VALUE obj = argv[0];
    return rb_funcallv(obj, rb_intern("new"), argc - 1, argv + 1);
}

VALUE sorbet_int_bool_true(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                           VALUE closure) {
    return Qtrue;
}

VALUE sorbet_int_bool_false(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                            VALUE closure) {
    return Qfalse;
}

VALUE sorbet_int_bool_and(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                          VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    if (argv[0] != Qnil && argv[0] != Qfalse) {
        return Qtrue;
    }
    return Qfalse;
}

VALUE sorbet_int_bool_nand(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                           VALUE closure) {
    sorbet_ensure_arity(argc, 1);
    if (argv[0] != Qnil && argv[0] != Qfalse) {
        return Qfalse;
    }
    return Qtrue;
}

// ****
// ****                       Used to implement inline caches
// ****

RUBY_EXTERN rb_serial_t ruby_vm_global_constant_state;
RUBY_EXTERN rb_serial_t ruby_vm_global_method_state;

SORBET_INLINE
rb_serial_t sorbet_getConstantEpoch() {
    return ruby_vm_global_constant_state;
}

SORBET_INLINE
rb_serial_t sorbet_getMethodEpoch() {
    return ruby_vm_global_method_state;
}

rb_serial_t sorbet_getClassSerial(VALUE obj) {
    return RCLASS_SERIAL(rb_class_of(obj));
}

// compiler is closely aware of layout of this struct
struct FunctionInlineCache {
    const rb_callable_method_entry_t *me;

    // used to compare for validity:
    // https://github.com/ruby/ruby/blob/97d75639a9970ce3868ba91a57be1856a3957711/vm_method.c#L822-L823
    rb_serial_t method_state;
    rb_serial_t class_serial;
};

void sorbet_inlineCacheInvalidated(VALUE recv, struct FunctionInlineCache *cache, ID mid) {
    // cargo cult https://git.corp.stripe.com/stripe-internal/ruby/blob/48bf9833/vm_eval.c#L289
    const rb_callable_method_entry_t *me;
    me = rb_callable_method_entry(CLASS_OF(recv), mid);
    if (!me) {
        // cargo cult https://git.corp.stripe.com/stripe-internal/ruby/blob/48bf9833/vm_eval.c#L304-L306
        rb_raise(rb_eRuntimeError, "unimplemented call with a missing method");
    }
    cache->me = me;
    cache->method_state = sorbet_getMethodEpoch();
    cache->class_serial = sorbet_getClassSerial(recv);
}

SORBET_ATTRIBUTE(noinline)
VALUE sorbet_callFuncWithCache(VALUE recv, ID func, int argc,
                               SORBET_ATTRIBUTE(noescape) const VALUE *const restrict argv,
                               struct FunctionInlineCache *cache) {
    if (UNLIKELY(sorbet_getMethodEpoch() != cache->method_state) ||
        UNLIKELY(sorbet_getClassSerial(recv) != cache->class_serial)) {
        sorbet_inlineCacheInvalidated(recv, cache, func);
    }
    return rb_vm_call(GET_EC(), recv, func, argc, argv, cache->me);
}

SORBET_INLINE
VALUE sorbet_callFuncProcWithCache(VALUE recv, ID func, int argc,
                                   SORBET_ATTRIBUTE(noescape) const VALUE *const restrict argv, VALUE proc,
                                   struct FunctionInlineCache *cache) {
    if (!NIL_P(proc)) {
        // this is an inlined version of vm_passed_block_handler_set(GET_EC(), proc);
        vm_block_handler_verify(proc);
        GET_EC()->passed_block_handler = proc;
    }

    return sorbet_callFuncWithCache(recv, func, argc, argv, cache);
}

struct sorbet_iterMethodArg {
    VALUE recv;
    ID func;
    int argc;
    const VALUE *argv;
    struct FunctionInlineCache *cache;
};

static VALUE sorbet_iterMethod(VALUE obj) {
    struct sorbet_iterMethodArg *arg = (struct sorbet_iterMethodArg *)obj;
    return sorbet_callFuncWithCache(arg->recv, arg->func, arg->argc, arg->argv, arg->cache);
}

SORBET_ATTRIBUTE(noinline)
VALUE sorbet_callFuncBlockWithCache(VALUE recv, ID func, int argc,
                                    SORBET_ATTRIBUTE(noescape) const VALUE *const restrict argv, BlockFFIType blockImpl,
                                    VALUE closure, struct FunctionInlineCache *cache) {
    struct sorbet_iterMethodArg arg;
    arg.recv = recv;
    arg.func = func;
    arg.argc = argc;
    arg.argv = argv;
    arg.cache = cache;

    return rb_iterate(sorbet_iterMethod, (VALUE)&arg, blockImpl, closure);
}

// ****
// ****                       Exceptions
// ****

VALUE sorbet_getTRetry() {
    return sorbet_getConstant("T::Private::Retry::RETRY", 24);
}

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

extern void rb_set_errinfo(VALUE);

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
void sorbet_raiseIfNotNil(VALUE exception) {
    if (exception == RUBY_Qnil) {
        return;
    }

    rb_exc_raise(exception);
}

// This is a function that can be used in place of any exception function, and does nothing except for return nil.
VALUE sorbet_blockReturnUndef(VALUE **pc, VALUE *iseq_encoded, VALUE closure) {
    return sorbet_rubyUndef();
}

// ****
// ****                       Compile-time only intrinsics. These should be eliminated by passes.
// ****

#ifdef SORBET_LLVM_PAYLOAD
// These forward declarations don't actually exist except in the LLVM IR we generate for each C extension,
// so this function fails to link when compiling the payload into libruby.so.
//
// We don't actually need this function to be present in that shared object, so we can omit it.

VALUE sorbet_i_getRubyClass(const char *const className, long classNameLen) __attribute__((const));
VALUE sorbet_i_getRubyConstant(const char *const className, long classNameLen) __attribute__((const));

VALUE __sorbet_only_exists_to_keep_functions_alive__() __attribute__((optnone)) {
    // this function will be nuked but it exists to keep forward definitions alive for clang
    return (long)&sorbet_i_getRubyClass + (long)&sorbet_i_getRubyConstant + (long)&sorbet_getConstantEpoch +
           (long)&sorbet_getConstant;
}

#endif
