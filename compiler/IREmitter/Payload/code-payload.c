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

extern rb_serial_t sorbet_getConstantEpoch();
extern VALUE sorbet_getConstant(const char *path, long pathLen);

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
           (long)&sorbet_getConstant;
}
