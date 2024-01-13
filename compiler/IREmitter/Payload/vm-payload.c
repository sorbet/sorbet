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
#include <signal.h>

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

typedef VALUE (*ExceptionFFIType)(VALUE **pc, VALUE closure, rb_control_frame_t *cfp);

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
    raise(SIGTRAP);
}

// ****
// ****                       Constants, Classes and Modules
// ****

VALUE sorbet_rb_cObject() {
    return rb_cObject;
}

static VALUE sorbet_constants;

// Register a value with the `sorbet_constants` array, and return the index that it occupies in that array. This
// indirection enables us to register one address with the garbage collector as a GC root, rather than one address per
// global constant we plan to hold. The downside here is the indirection through a ruby array, however the benefit is
// that we don't grow the linked list in the garbage collector that defines additional gc roots.
long sorbet_globalConstRegister(VALUE val) {
    if (UNLIKELY(sorbet_constants == 0)) {
        sorbet_constants = rb_ary_new();
        rb_gc_register_address(&sorbet_constants);
    }

    // NOTE: this is a big assumption about not running in a threaded context
    long idx = RARRAY_LEN(sorbet_constants);
    rb_ary_push(sorbet_constants, val);

    return idx;
}

static VALUE sorbet_globalConstFetch(long idx) {
    if (UNLIKELY(idx >= RARRAY_LEN(sorbet_constants))) {
        rb_raise(rb_eIndexError, "%ld is out of bounds for the sorbet_constants array (%ld)\n", idx,
                 RARRAY_LEN(sorbet_constants));
    }
    return RARRAY_AREF(sorbet_constants, idx);
}

// Lookup a hash literal in the global constants array, and duplicate it.
VALUE sorbet_globalConstDupHash(long idx) {
    VALUE hash = sorbet_globalConstFetch(idx);
    return rb_hash_dup(hash);
}

struct vm_ifunc *sorbet_globalConstFetchIfunc(long idx) {
    return (struct vm_ifunc *)sorbet_globalConstFetch(idx);
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
    rb_raise(rb_eTypeError, "%s: Expected type %s, got type %s with value %+" PRIsVALUE, castMethod, type,
             rb_obj_classname(value), value);
}

__attribute__((__noreturn__)) void sorbet_raiseArity(int argc, int min, int max) {
    rb_exc_raise(sorbet_rb_arity_error_new(argc, min, max));
}

VALUE sorbet_addMissingKWArg(VALUE missing, VALUE sym) {
    if (UNLIKELY(missing == RUBY_Qundef)) {
        missing = rb_ary_new();
    }

    rb_ary_push(missing, sym);
    return missing;
}

// from class.c
VALUE rb_keyword_error_new(const char *error, VALUE keys);

__attribute__((__noreturn__)) void sorbet_raiseMissingKeywords(VALUE missing) {
    rb_exc_raise(rb_keyword_error_new("missing", missing));
}

__attribute__((__noreturn__)) void sorbet_raiseCallDataExtraKeywords(int keyword_len, VALUE *keywords) {
    // This is not quite right, but we can fix that up later.
    VALUE missing = rb_ary_new();
    for (int i = 0; i < keyword_len; ++i) {
        rb_ary_push(missing, keywords[i]);
    }
    sorbet_raiseMissingKeywords(missing);
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

struct SorbetLineNumberInfo {
    int iseq_size;
    struct iseq_insn_info_entry *insns_info;
    VALUE *iseq_encoded;
};

void sorbet_initLineNumberInfo(struct SorbetLineNumberInfo *info, VALUE *iseq_encoded, int numLines) {
    // This is the table that tells us the hash entry for instruction types
    const void *const *table = rb_vm_get_insns_address_table();
    VALUE nop = (VALUE)(table[YARVINSN_nop]);

    info->iseq_size = numLines;
    info->insns_info = ALLOC_N(struct iseq_insn_info_entry, numLines);
    info->iseq_encoded = iseq_encoded;

    for (int i = 0; i < numLines; i++) {
        int lineno = i + 1;
        info->insns_info[i].line_no = lineno;

        // we fill iseq_encoded with NOP instructions; it only exists because it
        // has to match the length of insns_info.
        info->iseq_encoded[i] = nop;
    }
}

// NOTE: parent is the immediate parent frame, so for the rescue clause of a
// top-level method the parent would be the method iseq, but for a rescue clause
// nested within a rescue clause, it would be the outer rescue iseq.
//
// https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/compile.c#L5669-L5671
rb_iseq_t *sorbet_allocateRubyStackFrame(VALUE funcName, ID func, VALUE filename, VALUE realpath, unsigned char *parent,
                                         int iseqType, int startLine, struct SorbetLineNumberInfo *info, ID *locals,
                                         int numLocals, int stackMax) {
    // DO NOT ALLOCATE RUBY LEVEL OBJECTS HERE. All objects that are passed to
    // this function should be retained (for GC purposes) by something else.

    // ... but actually this line allocates and will not be retained by anyone else,
    // so we pin this object right here. TODO: This leaks memory
    rb_iseq_t *iseq = rb_iseq_new(0, funcName, filename, realpath, (rb_iseq_t *)parent, iseqType);
    rb_gc_register_mark_object((VALUE)iseq);

    // The SorbetLineNumberInfo that we pass in, and the way we set up the encoded
    // positions for the iseq, use absolute line numbers from the beginning of the
    // file.  This setup feeds in to tracking the PC in the control frame, and how
    // Ruby determines line numbers etc. in backtraces.
    //
    // However, there is a separate set of information for the actual location of
    // the function associated with the iseq, rb_iseq_constant_body.location.  Ruby
    // consults this information for things like Method#source_location.  This
    // information has been setup by rb_iseq_new, but we need to fixup the first
    // line of the function.
    iseq->body->location.first_lineno = INT2FIX(startLine);

    // NOTE: positions is freed by rb_iseq_insns_info_encode_positions
    unsigned int *positions = ALLOC_N(unsigned int, info->iseq_size);

    for (int i = 0; i < info->iseq_size; i++) {
        positions[i] = i;
    }

    iseq->body->insns_info.body = info->insns_info;
    iseq->body->insns_info.positions = positions;
    iseq->body->iseq_size = info->iseq_size;
    iseq->body->insns_info.size = info->iseq_size;
    rb_iseq_insns_info_encode_positions(iseq);

    iseq->body->iseq_encoded = info->iseq_encoded;

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

    if ((iseqType == ISEQ_TYPE_METHOD || iseqType == ISEQ_TYPE_CLASS || iseqType == ISEQ_TYPE_TOP) && numLocals > 0) {
        // this is a simplified version of `iseq_set_local_table` from
        // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/compile.c#L1767-L1789

        // allocate space for the names of the locals
        ID *ids = (ID *)ALLOC_N(ID, numLocals);

        memcpy(ids, locals, numLocals * sizeof(ID));

        iseq->body->local_table = ids;
        iseq->body->local_table_size = numLocals;
    }

    iseq->body->stack_max = stackMax;

    return iseq;
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

VALUE sorbet_vm_expandSplatIntrinsic(VALUE thing, VALUE before, VALUE after) {
    const VALUE obj = thing;
    const VALUE *elems;
    long len;
    bool have_array = RB_TYPE_P(thing, T_ARRAY);
    /* Compare vm_insnhelper.c:vm_expandarray.  We do things a little differently
     * because we don't use the Ruby stack as scratch space and we're making an
     * array nominally big enough to contain all the elements we need for a
     * destructuring assignment.  vm_expandarray is potentially called multiple
     * times in such situations.
     */
    if (!have_array && NIL_P(thing = rb_check_array_type(thing))) {
        thing = obj;
        elems = &thing;
        len = 1;
    } else {
        elems = RARRAY_CONST_PTR_TRANSIENT(thing);
        len = RARRAY_LEN(thing);
    }

    long needed = FIX2LONG(before) + FIX2LONG(after);
    long missing = needed - len;
    if (missing <= 0) {
        return have_array ? thing : rb_ary_new4(len, elems);
    }

    RB_GC_GUARD(thing);

    VALUE arr = rb_ary_new4(len, elems);
    for (long i = 0; i < missing; i++) {
        rb_ary_push(arr, RUBY_Qnil);
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
VALUE sorbet_blockReturnUndef(VALUE **pc, VALUE closure, rb_control_frame_t *cfp) {
    return RUBY_Qundef;
}

extern VALUE sorbet_getConstant(const char *path, long pathLen);

#define MOD_CONST_GET(path) sorbet_getConstant(path, sizeof(path) - 1)

static VALUE sigs_for_methods() {
    static VALUE sigs;
    if (UNLIKELY(sigs == 0)) {
        sigs = rb_hash_new();
        rb_gc_register_address(&sigs);
    }
    return sigs;
}

static VALUE sigs_for_self_methods() {
    static VALUE sigs;
    if (UNLIKELY(sigs == 0)) {
        sigs = rb_hash_new();
        rb_gc_register_address(&sigs);
    }
    return sigs;
}

// In Ruby terms, this is:
//  sig{params(isSelf: T::Boolean, method: Symbol, self: Module, arg: T.nilable(Symbol), block: T.untyped).void}
void sorbet_vm_register_sig(VALUE isSelf, VALUE method, VALUE self, VALUE arg, rb_block_call_func_t block) {
    VALUE methods = MOD_CONST_GET("T::Private::Methods");
    VALUE args[] = {self, arg};
    VALUE built_sig = rb_block_call(methods, rb_intern("_declare_sig"), 2, args, block, Qnil);

    // Store the sig someplace where we can get to it later.  This is complicated,
    // because we can have cases like:
    //
    // sig {...}
    // def some_method(...)
    // # sometime later
    // sig {...}
    // def some_method(...)
    //
    // and Sorbet will accept this (so long as the multiple definitions type-check).
    //
    // Nested method definitions are also problematic: Sorbet models the *sigs*
    // of all such definitions as occurring at the top-level, even if those signatures
    // are not applied until execution would reach the definition of the method, viz.
    //
    // sig {...} # sig 1
    // def some_method(...)
    //   sig {...} # sig 2
    //   def some_internal_method(...) # internal
    // # sometime later
    // sig {...} # sig 3
    // def some_internal_method(...) # external
    //
    // is internally modeled as:
    //
    // Sorbet::Private::Static.sig {...} # sig 1
    // Sorbet::Private::Static.sig {...} # sig 2
    // Sorbet::Private::Static.sig {...} # sig 3
    // # ...
    // def some_method(...)
    //   Sorbet::Private::Static.keep_def(:some_internal_method)
    // end
    //
    // Sorbet::Private::Static.keep_def(:some_internal_method)
    //
    // In such a situation, we should actually apply sig 3 to the external definition
    // and only use sig 2 for the internal definition, but it seems hard to come up
    // with data structures that would reflect that situation.
    //
    // There are disabled testcases for situations like this, and multiple definitions
    // generally, which the Sorbet compiler does not gracefully handle at the moment.
    //
    // For now, we're just going to say that for every (module, method_name)
    // combination, there always exists a unique signature that applies to the single
    // method defined as method_name.  This is obviously not correct, given the above,
    // but is good enough for our purposes.
    VALUE sig_table = RTEST(isSelf) ? sigs_for_self_methods() : sigs_for_methods();
    VALUE mod_entry = rb_hash_lookup2(sig_table, self, Qundef);
    if (mod_entry == Qundef) {
        mod_entry = rb_hash_new();
        rb_hash_aset(sig_table, self, mod_entry);
    }

    rb_hash_aset(mod_entry, method, built_sig);
}

struct method_block_params {
    VALUE klass;
    const char *name;
    // name as an ID.
    ID id;
    rb_sorbet_func_t methodPtr;
    // rb_sorbet_param_t
    void *paramp;
    rb_iseq_t *iseq;
    bool isSelf;
};

static VALUE define_method_block(RB_BLOCK_CALL_FUNC_ARGLIST(first_arg, data)) {
    struct method_block_params *params = (struct method_block_params *)data;
    if (params->isSelf) {
        rb_define_singleton_sorbet_method(params->klass, params->name, params->methodPtr, params->paramp, params->iseq);
    } else {
        rb_add_method_sorbet(params->klass, params->id, params->methodPtr, params->paramp, METHOD_VISI_PUBLIC,
                             params->iseq);
    }
    return Qnil;
}

void sorbet_vm_define_method(VALUE klass, const char *name, rb_sorbet_func_t methodPtr, void *paramp, rb_iseq_t *iseq,
                             bool isSelf) {
    VALUE sig_table = isSelf ? sigs_for_self_methods() : sigs_for_methods();
    VALUE mod_entry = rb_hash_lookup2(sig_table, klass, Qnil);
    VALUE built_sig = Qnil;
    ID id = rb_intern(name);
    if (mod_entry != Qnil) {
        built_sig = rb_hash_delete(mod_entry, ID2SYM(id));
    }

    struct method_block_params params;
    params.klass = klass;
    params.name = name;
    params.id = id;
    params.methodPtr = methodPtr;
    params.paramp = paramp;
    params.iseq = iseq;
    params.isSelf = isSelf;

    VALUE methods = MOD_CONST_GET("T::Private::Methods");
    VALUE args[] = {klass, built_sig};
    rb_block_call(methods, rb_intern("_with_declared_signature"), 2, args, define_method_block, (VALUE)&params);
}

static VALUE sorbet_getTPropsDecorator() {
    static const char decorator[] = "T::Props::Decorator";
    return sorbet_getConstant(decorator, sizeof(decorator));
}

/* See the patched proc.c */
extern VALUE sorbet_vm_method_owner(VALUE obj);

void sorbet_vm_define_prop_getter(VALUE klass, const char *name, rb_sorbet_func_t methodPtr, void *paramp,
                                  rb_iseq_t *iseq) {
    /* See T::Props::Decorator#define_getter_and_setter.  */
    ID prop_get = rb_intern("prop_get");
    VALUE decorator = rb_funcall(klass, rb_intern("decorator"), 0);
    VALUE prop_get_method = rb_obj_method(decorator, rb_id2sym(prop_get));
    VALUE method_owner = sorbet_vm_method_owner(prop_get_method);
    /* The code that the compiler generated was fully general, accessing instance variables
     * and going through any available prop_get_logic method.  In the case where prop_get
     * is known to be defined from T::Props::Decorator, we can use the attr_reader fastpath,
     * which is significantly faster.
     */
    if (method_owner == sorbet_getTPropsDecorator()) {
        ID method_name = rb_intern(name);
        ID attriv = rb_intern_str(rb_sprintf("@%" PRIsVALUE, rb_id2str(method_name)));
        rb_add_method(klass, method_name, VM_METHOD_TYPE_IVAR, (void *)attriv, METHOD_VISI_PUBLIC);
        return;
    }

    const bool isSelf = false;
    sorbet_vm_define_method(klass, name, methodPtr, paramp, iseq, isSelf);
}

// The layout of these structs is known to the compiler.
struct IDDescriptor {
    unsigned int offset;
    unsigned int length;
};

struct RubyStringDescriptor {
    unsigned int offset;
    unsigned int length;
};

void sorbet_vm_intern_ids(ID *idTable, struct IDDescriptor *idDescriptors, size_t numIDs, const char *stringTable) {
    for (size_t i = 0; i < numIDs; ++i) {
        const struct IDDescriptor *desc = &idDescriptors[i];
        idTable[i] = rb_intern2(&stringTable[desc->offset], desc->length);
    }
}

extern VALUE sorbet_vm_fstring_new(const char *ptr, long len);
extern void rb_gc_register_address(VALUE *addr);

void sorbet_vm_init_string_table(VALUE *rubyStringTable, struct RubyStringDescriptor *descriptors,
                                 size_t numRubyStrings, const char *stringTable) {
    for (size_t i = 0; i < numRubyStrings; ++i) {
        const struct RubyStringDescriptor *desc = &descriptors[i];
        rubyStringTable[i] = sorbet_vm_fstring_new(&stringTable[desc->offset], desc->length);
        // TODO(froydnj) This is going to allocate a separate linked list node for
        // every literal string in the program, which seems suboptimal.  We could
        // investigate a internal-only ruby object that knew how to "mark" itself
        // and marked every address in here during a GC?  Or add a special
        // rb_gc_register_address_range function?
        rb_gc_register_address(&rubyStringTable[i]);
    }
}
