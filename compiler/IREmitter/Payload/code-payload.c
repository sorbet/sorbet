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
