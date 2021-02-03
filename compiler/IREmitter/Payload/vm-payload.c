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

#define SORBET_ATTRIBUTE(...) __attribute__((__VA_ARGS__))

#define SORBET_INLINE SORBET_ATTRIBUTE(always_inline)

// Paul's and Dmitry's laptops have different attributes for this function in system libraries.
void abort(void) __attribute__((__cold__)) __attribute__((__noreturn__));

// Common definitions

typedef VALUE (*BlockFFIType)(VALUE firstYieldedArg, VALUE closure, int argCount, const VALUE *args, VALUE blockArg);

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
// ****                       Constants, Classes and Modules
// ****

VALUE sorbet_rb_cObject() {
    return rb_cObject;
}

// ****
// ****                       Calls
// ****

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

// ****
// **** Optimized versions of callFunc.
// **** Should use the same calling concention.
// **** Call it ending with `_no_type_guard` if implementation has a backed in slowpath
// ****
// ****

// ****
// ****                       Control Frames
// ****

/* From inseq.h */
struct iseq_insn_info_entry {
    int line_no;
    rb_event_flag_t events;
};

void rb_iseq_insns_info_encode_positions(const rb_iseq_t *iseq);
/* End from inseq.h */

// NOTE: parent is the immediate parent frame, so for the rescue clause of a
// top-level method the parent would be the method iseq, but for a rescue clause
// nested within a rescue clause, it would be the outer rescue iseq.
//
// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/compile.c#L5669-L5671
void *sorbet_allocateRubyStackFrame(VALUE funcName, ID func, VALUE filename, VALUE realpath, unsigned char *parent,
                                    int iseqType, int startline, int endline, ID *locals, int numLocals, int stackMax) {
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

    iseq->body->stack_max = stackMax;

    // Cast it to something easy since teaching LLVM about structs is a huge PITA
    return (void *)iseq;
}

const VALUE sorbet_readRealpath() {
    VALUE realpath = rb_gv_get("$__sorbet_ruby_realpath");
    if (!RB_TYPE_P(realpath, T_STRING)) {
        rb_raise(rb_eRuntimeError, "Invalid '$__sorbet_ruby_realpath' when loading compiled module");
    }

    rb_gv_set("$__sorbet_ruby_realpath", RUBY_Qnil);
    return realpath;
}

// ****
// ****                       Name Based Intrinsics
// ****

VALUE sorbet_splatIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                            VALUE closure) {
    if (UNLIKELY(argc != 3)) {
        sorbet_raiseArity(argc, 3, 3);
    }
    VALUE arr = argv[0];
    long len = RARRAY_LEN(arr);
    int size = FIX2LONG(argv[1]) + FIX2LONG(argv[2]);
    int missing = size - len;
    if (missing > 0) {
        VALUE newArr = rb_ary_dup(arr);
        for (int i = 0; i < missing; i++) {
            rb_ary_push(newArr, RUBY_Qnil);
        }
        return newArr;
    }
    return arr;
}

// ****
// ****                       Symbol Intrinsics. See CallCMethod in SymbolIntrinsics.cc
// ****

VALUE sorbet_enumerator_size_func_array_length(VALUE array, VALUE args, VALUE eobj) {
    return RARRAY_LEN(array);
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

// ****
// ****                       Exceptions
// ****

// This is a function that can be used in place of any exception function, and does nothing except for return nil.
VALUE sorbet_blockReturnUndef(VALUE **pc, VALUE *iseq_encoded, VALUE closure) {
    return RUBY_Qundef;
}
