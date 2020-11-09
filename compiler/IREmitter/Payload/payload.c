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
// ****                       Singletons
// ****

// use this undefined value when you have a variable that should _never_ escape to ruby.
//
// Note that we have `sorbet_rubyUndef` in the code payload, because the compiler
// occasionally needs to construct undefined values.  We have this here to call out
// the uses of `RUBY_Qundef` a little more noticeably.
SORBET_INLINE
static VALUE sorbet_payload_rubyUndef() {
    return RUBY_Qundef;
}

// ****
// ****                       Constants, Classes and Modules
// ****

VALUE sorbet_rb_cObject() {
    return rb_cObject;
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

    rb_gv_set("$__sorbet_ruby_realpath", RUBY_Qnil);
    return realpath;
}

// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm_insnhelper.c#L349-L359
SORBET_ATTRIBUTE(noinline)
void sorbet_vm_env_write_slowpath(const VALUE *ep, int index, VALUE v) {
    /* remember env value forcely */
    rb_gc_writebarrier_remember(VM_ENV_ENVVAL(ep));
    VM_FORCE_WRITE(&ep[index], v);
    VM_ENV_FLAGS_UNSET(ep, VM_ENV_FLAG_WB_REQUIRED);
}

// This is an inlined version of c_stack_overflow(GET_EC(), TRUE).
//
// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm_insnhelper.c#L34-L48
SORBET_ATTRIBUTE(__noreturn__)
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

extern void rb_vm_pop_frame(rb_execution_context_t *ec);

void sorbet_popRubyStack() {
    rb_vm_pop_frame(GET_EC());
}

// ****
// ****                       Implementation helpers for type tests
// ****

/*
_Bool sorbet_isa_Method(VALUE obj) __attribute__((const))  {
    return rb_obj_is_method(obj) == Qtrue;
}
*/

// ****
// ****                       Helpers for Intrinsics
// ****

void sorbet_ensure_arity_payload(int argc, int expected) {
    if (UNLIKELY(argc != expected)) {
        sorbet_raiseArity(argc, expected, expected);
    }
}

// ****
// ****                       Name Based Intrinsics
// ****

VALUE sorbet_splatIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                            VALUE closure) {
    sorbet_ensure_arity_payload(argc, 3);
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

// This doesn't do exactly the right thing because that is done by the parser in Ruby. Ruby will return the String
// "expression" if the RHS is an expression.
VALUE sorbet_definedIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                              VALUE closure) {
    if (argc == 0) {
        return RUBY_Qnil;
    }
    VALUE klass = sorbet_rb_cObject();
    for (int i = 0; i < argc; i++) {
        VALUE str = argv[i];
        ID id = rb_intern(RSTRING_PTR(str));
        if (!rb_const_defined_at(klass, id)) {
            return RUBY_Qnil;
        }
        klass = sorbet_getConstantAt(klass, id);
    }
    return rb_str_new2("constant");
}

// ****
// ****                       Symbol Intrinsics. See CallCMethod in SymbolIntrinsics.cc
// ****

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
VALUE sorbet_enumerator_size_func_array_length(VALUE array, VALUE args, VALUE eobj) {
    return RARRAY_LEN(array);
}

VALUE sorbet_rb_int_plus_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            return rb_fix_plus_fix(recv, y);
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_plus(y, recv);
        } else if (RB_TYPE_P(y, T_FLOAT)) {
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

VALUE sorbet_rb_int_gt_slowpath(VALUE recv, VALUE y) {
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

VALUE sorbet_rb_int_ge_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) >= FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_cmp(y, recv) == INT2FIX(+1) ? Qfalse : Qtrue;
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return rb_integer_float_cmp(recv, y) == INT2FIX(-1) ? Qfalse : Qtrue;
        }
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_ge(recv, y);
    }
    return rb_num_coerce_relop(recv, y, idGE);
}

VALUE sorbet_rb_int_le_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) <= FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_cmp(y, recv) == INT2FIX(-1) ? Qfalse : Qtrue;
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return rb_integer_float_cmp(recv, y) == INT2FIX(+1) ? Qfalse : Qtrue;
        }
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_le(recv, y);
    }
    return rb_num_coerce_relop(recv, y, idLE);
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
// ****                       Used to implement inline caches
// ****

RUBY_EXTERN rb_serial_t ruby_vm_global_method_state;

// Marked `static inline` because this is expected to only exist in the ruby vm
static inline rb_serial_t sorbet_getMethodEpoch() {
    return ruby_vm_global_method_state;
}

// Marked `static inline` because this is expected to only exist in the ruby vm
static inline rb_serial_t sorbet_getClassSerial(VALUE obj) {
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

// NOTE: kw_splat is 1 when keyword arguments are present in the argument list. This will be used in ruby-2.7.
SORBET_ATTRIBUTE(noinline)
VALUE sorbet_callFuncWithCache(VALUE recv, ID func, int argc,
                               SORBET_ATTRIBUTE(noescape) const VALUE *const restrict argv, int kw_splat,
                               struct FunctionInlineCache *cache) {
    if (UNLIKELY(sorbet_getMethodEpoch() != cache->method_state) ||
        UNLIKELY(sorbet_getClassSerial(recv) != cache->class_serial)) {
        sorbet_inlineCacheInvalidated(recv, cache, func);
    }
    return rb_vm_call(GET_EC(), recv, func, argc, argv, cache->me);
}

// ****
// ****                       Exceptions
// ****

// This is a function that can be used in place of any exception function, and does nothing except for return nil.
VALUE sorbet_blockReturnUndef(VALUE **pc, VALUE *iseq_encoded, VALUE closure) {
    return sorbet_payload_rubyUndef();
}
