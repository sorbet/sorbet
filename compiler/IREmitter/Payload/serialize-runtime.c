#include "id.h"
#include "internal.h"
#include "ruby.h"
#include "vm_core.h"

extern VALUE rb_mT;
extern VALUE rb_mT_Configuration;
extern VALUE rb_mT_Props;
extern VALUE rb_mT_Props_CustomType;
extern VALUE rb_mT_Props_Serializable;
extern VALUE rb_mT_Props_Utils;
extern VALUE rb_mT_Types;
extern VALUE rb_cT_Types_Enum;
extern VALUE rb_cT_Types_Intersection;
extern VALUE rb_cT_Types_Simple;
extern VALUE rb_cT_Types_TypedArray;
extern VALUE rb_cT_Types_TypedHash;
extern VALUE rb_cT_Types_TypedSet;
extern VALUE rb_cT_Types_Union;
extern VALUE rb_mT_Utils;
extern VALUE rb_mT_Utils_Nilable;

extern VALUE sorbet_vm_getivar(VALUE, ID, struct iseq_inline_iv_cache_entry *);
extern void sorbet_vm_setivar(VALUE, ID, VALUE, struct iseq_inline_iv_cache_entry *);

/* in our patched hash.c */
extern long sorbet_hash_string(VALUE a);

/* in our patched st.c */
extern int st_lookup_with_precomputed_hash(st_table *tab, st_data_t key, st_data_t *value, st_index_t hash);

/* TODO: add caching for returned modules */
#define rb_mod_const_get(mod, name) sorbet_getConstantAt(mod, rb_intern(name))

extern VALUE sorbet_getConstantAt(VALUE mod, ID id);

/* Ruby's object representation is a little strange.  Arrays (resp. hashes) have the
 * tag T_ARRAY (resp. T_HASH), which is fairly standard.  The unusual thing is that
 * *subclasses* of Array (resp. Hash) *also* have tag T_ARRAY.  So checking
 * RB_TYPE_P(obj, T_ARRAY) is not enough to know "is this object of type (exactly)
 * Array"; we also need to check the class of the object as well.
 *
 * We do all of this because we want to know whether it's "safe" to call the
 * underlying C functions for Array (resp. Hash) directly.  Note that we assume
 * that one is not redefining methods on Array (resp. Hash), but subclassing Array
 * (resp. Hash) and overriding methods is fine -- we'll just do regular method
 * dispatch in such cases.
 */
static __attribute__((always_inline)) _Bool can_directly_call_array_functions(VALUE);
static __attribute__((always_inline)) _Bool can_directly_call_hash_functions(VALUE);

static _Bool can_directly_call_array_functions(VALUE obj) {
    return LIKELY(RB_TYPE_P(obj, T_ARRAY)) && LIKELY(RBASIC(obj)->klass == rb_cArray);
}

static _Bool can_directly_call_hash_functions(VALUE obj) {
    return LIKELY(RB_TYPE_P(obj, T_HASH)) && LIKELY(RBASIC(obj)->klass == rb_cHash);
}

/* "Transforms" are translated from T::Props::Private::SerdeTransform.generate:
 *
 * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/private/serde_transform.rb#L27-L152
 *
 * step_for_type, below, is the translation of the above function into C.  Each
 * transform_* function roughly represents a when clause in the above method.
 * transform_step encapsulates both the function and (via its closure member) the
 * result of a recursive call to SerdeTransform.generate.
 *
 */
struct transform_step;

struct transform_ctx {
    /* Copied from the prop_descs_vec that we are dealing with.  */
    size_t n_modules;
    VALUE *modules;

    struct rb_call_data *caches;

    /* Propagating down the strict parameter of the top-level call.  */
    VALUE strict;
};
/* VALUE comes first so we can abuse the C calling convention and use e.g. rb_ary_dup
 * or rb_hash_dup in cases where that works.  */
typedef VALUE (*transform_fun)(VALUE, struct transform_ctx *, struct transform_step *);

/* This structure could be smaller:
 *
 * - fun should be an enum that selects the appropriate transform at runtime.
 *   ideally this should be faster and cut down on indirect call overhead.
 *   an enum for all the various transforms will also be smaller than a full
 *   pointer.
 * - idx should be 32-bit only, because we don't need 8 bytes and it packs better
 *   with making fun an enum.  Likewise for cache.
 * - there are probably ways to make the closure fields smaller, but maybe those
 *   are less necessary?
 */
struct transform_step {
    transform_fun fun;
    /* Index into a list of module names for when we're dealing with custom types or
     * subtypes of T::Props::Serializable.  */
    size_t idx;
    size_t cache;
    struct transform_step *closure;
    /* only necessary for transforming hashes when keys and values are not passthrough */
    struct transform_step *closure2;
};

static VALUE transform_passthrough(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    return x;
}

static VALUE transform_dup_array(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    if (UNLIKELY(!can_directly_call_array_functions(x))) {
        return rb_funcallv(x, rb_intern("dup"), 0, NULL);
    }

    return rb_ary_dup(x);
}

struct transform_block_closure {
    struct transform_ctx *ctx;
    struct transform_step *self;
};

static VALUE map_nonarray_block(VALUE block_arg, VALUE data, int argc, const VALUE *argv, VALUE block_as_proc) {
    struct transform_block_closure *closure = (struct transform_block_closure *)data;
    return closure->self->fun(argv[0], closure->ctx, closure->self);
}

static VALUE transform_map_array(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    /* subclasses might have overridden #[] */
    if (UNLIKELY(!can_directly_call_array_functions(x))) {
        struct transform_block_closure closure;
        closure.ctx = ctx;
        closure.self = self->closure;

        return rb_block_call(x, rb_intern("map"), 0, NULL, map_nonarray_block, (VALUE)&closure);
    }

    long len = RARRAY_LEN(x);
    VALUE mapped = rb_ary_new2(len);
    struct transform_step *inner = self->closure;

    for (long i = 0; i < len; ++i) {
        rb_ary_push(mapped, inner->fun(RARRAY_AREF(x, i), ctx, inner));
    }

    return mapped;
}

static VALUE transform_dup_set(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    return rb_funcallv(x, rb_intern("dup"), 0, NULL);
}

static VALUE map_set_block(VALUE block_arg, VALUE data, int argc, const VALUE *argv, VALUE block_as_proc) {
    struct transform_block_closure *closure = (struct transform_block_closure *)data;
    return closure->self->fun(argv[0], closure->ctx, closure->self);
}

static VALUE transform_map_set(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    /* TODO: cache the lookup of Set */
    VALUE Set = rb_mod_const_get(rb_cObject, "Set");

    struct transform_block_closure closure;
    closure.ctx = ctx;
    closure.self = self->closure;

    return rb_block_call(Set, rb_intern("new"), 1, &x, map_set_block, (VALUE)&closure);
}

/* TODO: consider implementing these via rb_hash_foreach to avoid block allocation
 * and calling overhead.
 */
static VALUE each_hash_block(VALUE block_arg, VALUE data, int argc, const VALUE *argv, VALUE block_as_proc) {
    struct transform_block_closure *closure = (struct transform_block_closure *)data;
    VALUE ary = argv[0];
    VALUE k = RARRAY_AREF(ary, 0);
    VALUE v = RARRAY_AREF(ary, 1);
    VALUE h = argv[1];
    struct transform_step *s1 = closure->self->closure;
    struct transform_step *s2 = closure->self->closure2;
    return rb_hash_aset(h, s1->fun(k, closure->ctx, s1), s2->fun(v, closure->ctx, s2));
}

static VALUE transform_each_hash(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    // If we are transforming a hash, we can at least try to size it correctly.
    VALUE transformed;

    if (LIKELY(can_directly_call_hash_functions(x))) {
        transformed = rb_hash_new_with_size(RHASH_SIZE(x));
    } else {
        transformed = rb_hash_new();
    }

    /* We are explicitly passing `self` here rather than `self->closure`, because we
     * want both `self->closure` and `self->closure2` to be available in `each_hash_block`.
     */
    struct transform_block_closure closure;
    closure.ctx = ctx;
    closure.self = self;

    return rb_block_call(x, rb_intern("each_with_object"), 1, &transformed, each_hash_block, (VALUE)&closure);
}

static VALUE keys_block(VALUE block_arg, VALUE data, int argc, const VALUE *argv, VALUE block_as_proc) {
    struct transform_block_closure *closure = (struct transform_block_closure *)data;
    return closure->self->fun(argv[0], closure->ctx, closure->self);
}

static VALUE transform_keys_hash(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    struct transform_block_closure closure;
    closure.ctx = ctx;
    closure.self = self->closure;

    return rb_block_call(x, rb_intern("transform_keys"), 0, NULL, keys_block, (VALUE)&closure);
}

static VALUE values_nonhash_block(VALUE block_arg, VALUE data, int argc, const VALUE *argv, VALUE block_as_proc) {
    struct transform_block_closure *closure = (struct transform_block_closure *)data;
    return closure->self->fun(argv[0], closure->ctx, closure->self);
}

struct transform_values_closure {
    struct transform_ctx *ctx;
    struct transform_step *self;
    VALUE hash;
};

/* This function is called for each entry in the hashtable to determine whether
 * we want to replace some aspect of the entry.  Since we do, we return ST_REPLACE.
 */
static int transform_values_foreach_check(st_data_t key, st_data_t value, st_data_t argp, int error) {
    return ST_REPLACE;
}

/* This function is called for each entry in the hashtable to change the entry
 * in some way.  For transforming values, we want to modify the value stored in
 * the hashtable, and then return ST_CONTINUE to indicate that we should continuing
 * iterating over other entries in the table.
 */
static int transform_values_foreach_replace(st_data_t *key, st_data_t *value, st_data_t argp, int error) {
    struct transform_values_closure *closure = (struct transform_values_closure *)argp;
    VALUE new_val = closure->self->fun(*value, closure->ctx, closure->self);
    RB_OBJ_WRITE(closure->hash, value, new_val);
    return ST_CONTINUE;
}

static VALUE transform_values_hash(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    if (UNLIKELY(!can_directly_call_hash_functions(x))) {
        struct transform_block_closure closure;
        closure.ctx = ctx;
        closure.self = self->closure;

        return rb_block_call(x, rb_intern("transform_values"), 0, NULL, values_nonhash_block, (VALUE)&closure);
    }

    VALUE ret = rb_hash_dup(x);

    struct transform_values_closure closure;
    closure.ctx = ctx;
    closure.self = self->closure;
    closure.hash = ret;

    rb_hash_stlike_foreach_with_replace(ret, transform_values_foreach_check, transform_values_foreach_replace,
                                        (st_data_t)&closure);

    return ret;
}

static VALUE transform_dup_hash(VALUE x, struct transform_ctx *ctx, struct transform_step *inner) {
    if (UNLIKELY(!can_directly_call_hash_functions(x))) {
        return rb_funcallv(x, rb_intern("dup"), 0, NULL);
    }

    return rb_hash_dup(x);
}

static VALUE transform_to_float(VALUE x, struct transform_ctx *ctx, struct transform_step *inner) {
    /* TODO: pick off the easy Float, Integer, String cases before doing a full dispatch */
    return rb_funcallv(x, idTo_f, 0, NULL);
}

static VALUE transform_with_nil_check(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    VALUE nil_p = rb_funcallv(x, idNilP, 0, NULL);
    if (RTEST(nil_p)) {
        return Qnil;
    }

    struct transform_step *inner = self->closure;
    return inner->fun(x, ctx, inner);
}

static VALUE transform_call_serialize(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    return rb_funcallv(x, rb_intern("serialize"), 1, &ctx->strict);
}

static VALUE transform_call_checked_serialize(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    return rb_funcallv(rb_mT_Props_CustomType, rb_intern("checked_serialize"), 1, &x);
}

static VALUE transform_deep_clone_object(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    return rb_funcall(rb_mT_Props_Utils, rb_intern("deep_clone_object"), 1, x);
}

static VALUE transform_call_from_hash(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    return rb_funcallv_with_cc(&ctx->caches[self->cache], ctx->modules[self->idx], rb_intern("from_hash"), 1, &x);
}

static VALUE transform_call_deserialize(VALUE x, struct transform_ctx *ctx, struct transform_step *self) {
    return rb_funcallv_with_cc(&ctx->caches[self->cache], ctx->modules[self->idx], rb_intern("deserialize"), 1, &x);
}

/* This struct stores data we accumulate while we're deciding how to serialize/deserialize
 * individual types.
 */
struct serde_ctx {
    /* Types that we need to record for serde on serializable and custom types
     * and later pass into the actual serde process.
     */
    size_t module_index;
    VALUE module_array;

    size_t call_cache_index;
    /* IDs (as symbols) representing the method to be called for the particular
     * call cache being reserved.
     */
    VALUE call_cache_array;
};

static struct transform_step *step_new_priv(transform_fun fun, size_t idx, size_t cache, struct transform_step *closure,
                                            struct transform_step *closure2) {
    struct transform_step *step = malloc(sizeof(*step));
    step->fun = fun;
    step->idx = idx;
    step->cache = cache;
    step->closure = closure;
    step->closure2 = closure2;
    return step;
}

static struct transform_step *step_new2(transform_fun fun, struct transform_step *closure,
                                        struct transform_step *closure2) {
    return step_new_priv(fun, 0, 0, closure, closure2);
}

static struct transform_step *step_new(transform_fun fun, struct transform_step *closure) {
    return step_new_priv(fun, 0, 0, closure, NULL);
}

static struct transform_step *step_new_indexed(transform_fun fun, size_t idx, size_t cache) {
    return step_new_priv(fun, idx, cache, NULL, NULL);
}

static void step_free(struct transform_step *step) {
    if (step->closure != NULL) {
        step_free(step->closure);
    }
    if (step->closure2 != NULL) {
        step_free(step->closure2);
    }
    free(step);
}

static _Bool step_is_passthrough(struct transform_step *step) {
    return step->fun == transform_passthrough;
}

/* When generating Ruby code, sorbet-runtime can just compare the string representations
 * of transforms on particular values to determine whether the transforms are equalish.
 * Note that the comparison is not necessarily checking whether identical types are
 * being transformed: integers and strings both receive nil transforms in
 * sorbet-runtime -- transform_passthough here -- but would generally compare as equal
 * in the places where equalish matters.
 *
 * We don't have strings available, so we have to do something slightly different.
 */
static _Bool step_equalish(struct serde_ctx *ctx, struct transform_step *s1, struct transform_step *s2) {
    if (s1 == NULL && s2 == NULL) {
        return true;
    }

    if (s1 == NULL || s2 == NULL) {
        return false;
    }

    if (s1->fun != s2->fun) {
        return false;
    }

    /* We also need to compare the modules used by s1 and s2: deserializing different
     * custom types (transform_call_deserialize) should compare as different despite
     * using the same transform function.  (In sorbet-runtime, the module names would
     * appear in the string representation of the transform, so they would obviously
     * compare differently.)
     *
     * We don't need to concern ourselves deciding if s1 and s2 will actually use a
     * module at runtime:
     *
     * - if s1 doesn't, then s1->idx will be 0 and so will s2->idx.
     * - if s1 does, then s1->idx will be equal to s2->idx *or* they refer to the
     *   the same module in the module array.  Since modules compare equal by pointer
     *   equality, we can compare them thus here.
     *
     * TODO: this is a bit clunky; we should really ensure that any given module
     * is only recorded once in serde_ctx.
     */
    if (s1->idx != s2->idx) {
        if (rb_ary_entry(ctx->module_array, s1->idx) != rb_ary_entry(ctx->module_array, s2->idx)) {
            return false;
        }
    }

    /* We do not need to compare call caches, since those are essentially private
     * runtime details.
     */

    return step_equalish(ctx, s1->closure, s2->closure) && step_equalish(ctx, s1->closure2, s2->closure2);
}

enum StepMode {
    Serialize,
    Deserialize,
};

static size_t push_module(struct serde_ctx *ctx, VALUE klass) {
    rb_ary_push(ctx->module_array, klass);
    return ctx->module_index++;
}

static size_t reserve_call_cache(struct serde_ctx *ctx, ID id) {
    rb_ary_push(ctx->call_cache_array, rb_id2sym(id));
    return ctx->call_cache_index++;
}

/* type: Module */
static struct transform_step *handle_serializable_subtype(struct serde_ctx *ctx, VALUE type, enum StepMode mode) {
    switch (mode) {
        case Serialize:
            return step_new(transform_call_serialize, NULL);
        case Deserialize: {
            size_t idx = push_module(ctx, type);
            size_t cache = reserve_call_cache(ctx, rb_intern("from_hash"));
            return step_new_indexed(transform_call_from_hash, idx, cache);
        }
    }
}

/* type: Module */
static struct transform_step *handle_custom_type(struct serde_ctx *ctx, VALUE type, enum StepMode mode) {
    switch (mode) {
        case Serialize:
            return step_new(transform_call_checked_serialize, NULL);
        case Deserialize: {
            size_t idx = push_module(ctx, type);
            size_t cache = reserve_call_cache(ctx, rb_intern("deserialize"));
            return step_new_indexed(transform_call_deserialize, idx, cache);
        }
    }
}

/* This function closely mirrors T::Props::Private::SerdeTransform.generate.
 * Comments throughout the function attempt to orient the reader to where they
 * are within `generate`.
 *
 * type: T::Types::Base
 */
static struct transform_step *step_for_type(struct serde_ctx *ctx, VALUE type, enum StepMode mode) {
    if (RTEST(rb_obj_is_kind_of(type, rb_cT_Types_TypedArray))) {
        VALUE inner_type = rb_funcallv(type, rb_intern("type"), 0, NULL);
        struct transform_step *inner_step = step_for_type(ctx, inner_type, mode);

        if (step_is_passthrough(inner_step)) {
            /*assert(inner_step->closure == NULL);*/
            inner_step->fun = transform_dup_array;
            return inner_step;
        }

        return step_new(transform_map_array, inner_step);
    }

    if (RTEST(rb_obj_is_kind_of(type, rb_cT_Types_TypedSet))) {
        VALUE inner_type = rb_funcallv(type, rb_intern("type"), 0, NULL);
        struct transform_step *inner_step = step_for_type(ctx, inner_type, mode);

        if (step_is_passthrough(inner_step)) {
            /*assert(inner_step->closure == NULL);*/
            inner_step->fun = transform_dup_set;
            return inner_step;
        }

        return step_new(transform_map_set, inner_step);
    }

    if (RTEST(rb_obj_is_kind_of(type, rb_cT_Types_TypedHash))) {
        VALUE keys = rb_funcallv(type, rb_intern("keys"), 0, NULL);
        VALUE values = rb_funcallv(type, rb_intern("values"), 0, NULL);
        struct transform_step *keys_step = step_for_type(ctx, keys, mode);
        struct transform_step *values_step = step_for_type(ctx, values, mode);
        _Bool passthrough_keys = step_is_passthrough(keys_step);
        _Bool passthrough_values = step_is_passthrough(values_step);

        if (!passthrough_keys && !passthrough_values) {
            return step_new2(transform_each_hash, keys_step, values_step);
        }

        if (!passthrough_keys) {
            step_free(values_step);
            return step_new(transform_keys_hash, keys_step);
        }

        if (!passthrough_values) {
            step_free(keys_step);
            return step_new(transform_values_hash, values_step);
        }

        /*assert(keys_step->closure == NULL);*/
        keys_step->fun = transform_dup_hash;
        step_free(values_step);

        return keys_step;
    }

    if (RTEST(rb_obj_is_kind_of(type, rb_cT_Types_Simple))) {
        VALUE raw = rb_funcallv(type, rb_intern("raw_type"), 0, NULL);
        /* NO_TRANSFORM_TYPES.any? {|cls| raw <= cls} */
        if (RTEST(rb_class_inherited_p(raw, rb_cTrueClass)) || RTEST(rb_class_inherited_p(raw, rb_cFalseClass)) ||
            RTEST(rb_class_inherited_p(raw, rb_cNilClass)) || RTEST(rb_class_inherited_p(raw, rb_cSymbol)) ||
            RTEST(rb_class_inherited_p(raw, rb_cString))) {
            return step_new(transform_passthrough, NULL);
        }

        /* raw <= Float */
        if (RTEST(rb_class_inherited_p(raw, rb_cFloat))) {
            switch (mode) {
                case Deserialize:
                    return step_new(transform_to_float, NULL);
                case Serialize:
                    return step_new(transform_passthrough, NULL);
            }
        }

        /* raw <= Numeric */
        if (RTEST(rb_class_inherited_p(raw, rb_cNumeric))) {
            return step_new(transform_passthrough, NULL);
        }

        /* raw < T::Props::Serializable */
        if (raw != rb_mT_Props_Serializable && RTEST(rb_class_inherited_p(raw, rb_mT_Props_Serializable))) {
            return handle_serializable_subtype(ctx, raw, mode);
        }

        /* raw < T::Props::CustomType */
        if (raw != rb_mT_Props_CustomType &&
            RTEST(rb_class_inherited_p(rb_singleton_class(raw), rb_mT_Props_CustomType))) {
            return handle_custom_type(ctx, raw, mode);
        }

        /* T::Configuration.scalar_types.include?(raw.name) */
        VALUE raw_name = rb_funcallv(raw, rb_intern("name"), 0, NULL);
        VALUE scalar_types = rb_funcallv(rb_mT_Configuration, rb_intern("scalar_types"), 0, NULL);
        VALUE includes_p = rb_funcallv(scalar_types, rb_intern("include?"), 1, &raw_name);
        if (RTEST(includes_p)) {
            return step_new(transform_passthrough, NULL);
        }

        /* else */
        return step_new(transform_deep_clone_object, NULL);
    }

    if (RTEST(rb_obj_is_kind_of(type, rb_cT_Types_Union))) {
        VALUE non_nil_type = rb_funcall(rb_mT_Utils, rb_intern("unwrap_nilable"), 1, type);
        if (RTEST(non_nil_type)) {
            struct transform_step *inner = step_for_type(ctx, non_nil_type, mode);
            if (step_is_passthrough(inner)) {
                return inner;
            }

            return step_new(transform_with_nil_check, inner);
        }

        /* type.types.all? {|t| generate(t, mode, varname).nil?} */
        VALUE types = rb_funcallv(type, rb_intern("types"), 0, NULL);
        bool all = true;
        for (long i = 0; i < RARRAY_LEN(types); ++i) {
            VALUE t = RARRAY_AREF(types, i);
            struct transform_step *inner = step_for_type(ctx, t, mode);
            all = step_is_passthrough(inner);
            step_free(inner);

            if (!all) {
                break;
            }
        }

        /* all types are passthrough, so the union is passthrough */
        if (all) {
            return step_new(transform_passthrough, NULL);
        }

        /* else */
        return step_new(transform_deep_clone_object, NULL);
    }

    if (RTEST(rb_obj_is_kind_of(type, rb_cT_Types_Intersection))) {
        VALUE types = rb_funcallv(type, rb_intern("types"), 0, NULL);
        struct transform_step *uniq = NULL;

        for (long i = 0; i < RARRAY_LEN(types); ++i) {
            VALUE t = RARRAY_AREF(types, i);
            struct transform_step *inner = step_for_type(ctx, t, mode);

            /* reject {|t| t == dynamic_fallback} */
            if (inner->fun == transform_deep_clone_object) {
                step_free(inner);
                continue;
            }

            if (uniq == NULL) {
                uniq = inner;
                continue;
            }

            /* Now check to see if some intermediate step is equalish to the
             * first non-dynamic fallback step we had.  If they are, then we
             * effectively still have only one (`uniq`) transform for all
             * the members of the intersection.  If they are not, then we
             * have two different cases and should stop immediately.
             */
            if (step_equalish(ctx, uniq, inner)) {
                step_free(inner);
                continue;
            }

            /* We have multiple cases and they weren't consistent. */
            step_free(uniq);
            step_free(inner);
            return step_new(transform_deep_clone_object, NULL);
        }

        /* We couldn't tell what to do, just fallback. */
        if (uniq == NULL) {
            return step_new(transform_deep_clone_object, NULL);
        }

        /* This is effectively the inner_known.first case. */
        return uniq;
    }

    if (RTEST(rb_obj_is_kind_of(type, rb_cT_Types_Enum))) {
        VALUE lifted = rb_funcallv(rb_mT_Utils, rb_intern("lift_enum"), 1, &type);

        return step_for_type(ctx, lifted, mode);
    }

    return step_new(transform_deep_clone_object, NULL);
}

/* This function is meant to implement the same logic as:
 *
 * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/private/serializer_generator.rb#L29-L39
 * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/private/deserializer_generator.rb#L34-L44
 */
static void validate_prop(VALUE prop, VALUE rules, VALUE *hash_key_ptr, ID *ivar_id_ptr) {
    VALUE T_Props_Decorator_SAFE_NAME = rb_mod_const_get(rb_cObject, "T::Props::Decorator::SAFE_NAME");
    VALUE prop_to_s = rb_funcallv(prop, idTo_s, 0, NULL);

    VALUE matched = rb_funcallv(T_Props_Decorator_SAFE_NAME, rb_intern("match"), 1, &prop_to_s);
    if (!RTEST(matched)) {
        rb_raise(rb_eRuntimeError, "invalid property name");
    }

    VALUE hash_key = rb_hash_fetch(rules, RB_ID2SYM(rb_intern("serialized_form")));
    matched = rb_funcallv(T_Props_Decorator_SAFE_NAME, rb_intern("match"), 1, &hash_key);
    if (!RTEST(matched)) {
        rb_raise(rb_eRuntimeError, "invalid serialized_form");
    }

    /* Note that {de,}serializer_generator.rb uses to_s on the accessor_key to obtain
     * something that works as Ruby source.  We use the symbol (actually, the ID of
     * the symbol) directly during the {de,}serialization process, but we implement
     * the checks on the string representation.
     */
    VALUE accessor_key = rb_hash_fetch(rules, RB_ID2SYM(rb_intern("accessor_key")));
    /* Defend against sorbet-runtime representation changes. */
    if (!RB_SYMBOL_P(accessor_key)) {
        rb_raise(rb_eRuntimeError, "expected accessor_key to be a symbol");
    }

    VALUE ivar_name_str = rb_funcallv(accessor_key, idTo_s, 0, NULL);
    VALUE at_str = rb_str_new_static("@", 1);
    VALUE starts_with = rb_funcallv(ivar_name_str, rb_intern("start_with?"), 1, &at_str);
    if (!RTEST(starts_with)) {
        rb_raise(rb_eRuntimeError, "invalid accessor_key");
    }

    const int exclude_end = 0;
    VALUE range = rb_range_new(LONG2FIX(1), LONG2FIX(-1), exclude_end);
    VALUE ivar_subseq = rb_funcallv(ivar_name_str, rb_intern("[]"), 1, &range);
    matched = rb_funcallv(T_Props_Decorator_SAFE_NAME, rb_intern("match"), 1, &ivar_subseq);
    if (!RTEST(matched)) {
        rb_raise(rb_eRuntimeError, "invalid accessor_key");
    }

    *hash_key_ptr = hash_key;
    /* Safe because we checked RB_SYMBOL_P above. */
    *ivar_id_ptr = RB_SYM2ID(accessor_key);
}

struct prop_desc;
typedef VALUE (*nil_handler)(VALUE self, struct prop_desc *desc);

/* A prop_desc contains all the information we need to serialize or deserialize a given
 * prop.  We will store separate arrays of prop_descs for serialization and deserialization
 * in class variables.
 */
struct prop_desc {
    /* the ID of the instance variable we are dealing with */
    ID ivar_id;

    /* instance variable access cache */
    struct iseq_inline_iv_cache_entry iv_cache;

    /* precomputed hash value for hash_key */
    st_index_t precomputed_hash;

    /* string key for the prop's serialized value */
    VALUE hash_key;

    /* how to transform a value, whether for serialization or deserialization */
    struct transform_step *transform;

    /* what to do in case of a nil when deserializing; NULL for serialization */
    nil_handler on_nil;

    /* the prop name we're dealing with, as a symbol */
    VALUE prop;

    /* default value for deserialization, if it matters */
    VALUE default_;

    /* holds the result of RTEST(rules[:fully_optional]) */
    bool fully_optional;
};

/* prop: Symbol
 * default_: a Ruby literal if the prop has such a default, Qnil otherwise
 * hash_key: String
 */
static void desc_new(struct prop_desc *desc, ID ivar_id, bool fully_optional, VALUE prop, VALUE default_,
                     VALUE hash_key, struct transform_step *transform, nil_handler handler) {
    desc->ivar_id = ivar_id;
    desc->fully_optional = fully_optional;
    desc->prop = prop;
    desc->default_ = default_;
    desc->hash_key = hash_key;
    desc->precomputed_hash = sorbet_hash_string(hash_key);
    desc->transform = transform;
    desc->on_nil = handler;
}

struct prop_descs_vec {
    /* Modules for custom deserialization or subtypes of Serialiazable.  Transform
     * steps for such types hold indices into this array.  */
    size_t n_modules;
    VALUE *modules;

    /* Call caches.  Transform steps that call Ruby methods hold indices into
     * this array.  */
    size_t n_caches;
    struct rb_call_data *caches;

    /* The actual property descriptors.  */
    size_t n_descs;
    struct prop_desc *descs;
};

/* Transfer accumulated context information such as the modules for custom
 * serialization/deserialization or call caches from `ctx` into `vec`; we have already
 * allocated the property descriptors for `vec`.
 */
static void transfer_context(struct serde_ctx *ctx, struct prop_descs_vec *vec) {
    long len = RARRAY_LEN(ctx->module_array);
    if (len == 0) {
        vec->n_modules = 0;
        vec->modules = NULL;
    } else {
        vec->n_modules = len;
        vec->modules = calloc(sizeof(VALUE), len);
        for (long i = 0; i < len; ++i) {
            vec->modules[i] = RARRAY_AREF(ctx->module_array, i);
        }
    }

    if (ctx->call_cache_index == 0) {
        vec->n_caches = 0;
        vec->caches = NULL;
    } else {
        vec->n_caches = ctx->call_cache_index;
        vec->caches = calloc(sizeof(*vec->caches), ctx->call_cache_index);
        for (long i = 0; i < ctx->call_cache_index; ++i) {
            vec->caches[i].ci.mid = rb_sym2id(RARRAY_AREF(ctx->call_cache_array, i));
        }
    }
}

/* This function and the interface it implements exists only to provide accounting
 * for ObjectSpace (`require 'objspace'`); it does not interact with the GC in any
 * way.  Therefore, it does not need to be absolutely precise; we account here for
 * the various vectors we track, but we do not attempt to measure the memory taken
 * by the transform needed by each prop.
 */
static size_t prop_descs_vec_memsize(const void *data) {
    const struct prop_descs_vec *vec = data;
    const size_t self_size = sizeof(*vec);
    const size_t modules_size = sizeof(*vec->modules) * vec->n_modules;
    const size_t caches_size = sizeof(*vec->caches) * vec->n_caches;
    const size_t descs_size = sizeof(*vec->descs) * vec->n_descs;
    return self_size + modules_size + descs_size;
}

static void prop_descs_vec_mark(void *data) {
    struct prop_descs_vec *vec = data;
    for (size_t i = 0; i < vec->n_descs; ++i) {
        struct prop_desc *desc = &vec->descs[i];
        rb_gc_mark(desc->prop);
        rb_gc_mark(desc->default_);
        rb_gc_mark(desc->hash_key);
    }
    for (size_t i = 0; i < vec->n_modules; ++i) {
        rb_gc_mark(vec->modules[i]);
    }
}

static void prop_descs_vec_free(void *data) {
    struct prop_descs_vec *vec = data;
    for (size_t i = 0; i < vec->n_descs; ++i) {
        struct prop_desc *desc = &vec->descs[i];
        step_free(desc->transform);
    }
    free(vec->modules);
    free(vec->caches);
    free(vec->descs);
    free(vec);
}

static const rb_data_type_t prop_descs_vec_type = {
    .wrap_struct_name = "prop_descs_vec",
    .function =
        {
            .dmark = prop_descs_vec_mark,
            .dfree = prop_descs_vec_free,
            .dsize = prop_descs_vec_memsize,
        },
    .data = NULL,
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

struct serialize_descs_closure {
    struct serde_ctx ctx;
    struct prop_descs_vec *descs_array;
    size_t i;
};

/* prop: Symbol
 * rules: T::Hash[Symbol, T.untyped]
 * arg: a `struct deserialize_desc_closure *` masquerading as a Ruby VALUE
 */
static int serialize_desc_for_prop(VALUE prop, VALUE rules, VALUE arg) {
    struct serialize_descs_closure *closure = (struct serialize_descs_closure *)arg;
    VALUE hash_key;
    ID ivar_id;

    validate_prop(prop, rules, &hash_key, &ivar_id);

    VALUE type_object = rb_hash_fetch(rules, RB_ID2SYM(rb_intern("type_object")));
    VALUE underlying_type = rb_funcallv(rb_mT_Utils_Nilable, rb_intern("get_underlying_type_object"), 1, &type_object);

    struct transform_step *step = step_for_type(&closure->ctx, underlying_type, Serialize);

    VALUE default_doesnt_matter_for_serialize = Qnil;
    nil_handler no_handler_for_serialize = NULL;
    VALUE fully_optional = rb_hash_aref(rules, RB_ID2SYM(rb_intern("fully_optional")));
    struct prop_desc *desc = &closure->descs_array->descs[closure->i++];
    desc_new(desc, ivar_id, RTEST(fully_optional), prop, default_doesnt_matter_for_serialize, hash_key, step,
             no_handler_for_serialize);

    return ST_CONTINUE;
}

/* Implement
 *
 * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/private/deserializer_generator.rb#L105-L163
 *
 * but in C, with individual functions standing in for the place of textual strings
 * that are returned by the deserialization generator.
 */
static VALUE on_nil_raise_nil_deserialize_error(VALUE self, struct prop_desc *desc) {
    VALUE klass = rb_funcallv(self, rb_intern("class"), 0, NULL);
    VALUE decorator = rb_funcallv(klass, rb_intern("decorator"), 0, NULL);
    VALUE inspected = rb_funcallv(desc->hash_key, rb_intern("inspect"), 0, NULL);
    return rb_funcallv(decorator, rb_intern("raise_nil_deserialize_error"), 1, &inspected);
}

static VALUE on_nil_literal_string(VALUE self, struct prop_desc *desc) {
    return rb_str_resurrect(desc->default_);
}

static VALUE on_nil_literal(VALUE self, struct prop_desc *desc) {
    return desc->default_;
}

static VALUE on_nil_fetch_default(VALUE self, struct prop_desc *desc) {
    VALUE klass = rb_funcallv(self, rb_intern("class"), 0, NULL);
    VALUE decorator = rb_funcallv(klass, rb_intern("decorator"), 0, NULL);
    VALUE props_with_defaults = rb_funcallv(decorator, rb_intern("props_with_defaults"), 0, NULL);
    VALUE fetched_prop = rb_funcallv(props_with_defaults, rb_intern("fetch"), 1, &desc->prop);
    return rb_funcallv(fetched_prop, rb_intern("default"), 0, NULL);
}

static VALUE on_nil_empty_array(VALUE self, struct prop_desc *desc) {
    return rb_ary_new();
}

static VALUE on_nil_empty_hash(VALUE self, struct prop_desc *desc) {
    return rb_hash_new();
}

static VALUE on_nil_required_prop_missing_from_deserialize(VALUE self, struct prop_desc *desc) {
    return rb_funcallv(self, rb_intern("required_prop_missing_from_deserialize"), 1, &desc->prop);
}

static VALUE on_nil_nil(VALUE self, struct prop_desc *desc) {
    return Qnil;
}

/* The actual logic of DeserializerGenerator.generate_nil_handler.
 *
 * default_: T.nilable(T::Props::Private::ApplyDefault)
 * nilable_type: T::Boolean
 * raise_on_nil_write:: T::Boolean
 * literal_default: outparam for a defaulted prop with a literal default value
 */
static nil_handler generate_nil_handler(VALUE default_, VALUE nilable_type, VALUE raise_on_nil_write,
                                        VALUE *literal_default) {
    if (!RTEST(nilable_type)) {
        /* when NilClass */
        if (default_ == Qnil) {
            return on_nil_raise_nil_deserialize_error;
        }

        /* when ApplyPrimitiveDefault */
        VALUE ApplyPrimitiveDefault =
            sorbet_getConstantAt(rb_cObject, rb_intern("T::Props::Private::ApplyPrimitiveDefault"));
        if (RTEST(rb_obj_is_kind_of(default_, ApplyPrimitiveDefault))) {
            VALUE literal = rb_funcallv(default_, rb_intern("default"), 0, NULL);
            /* when String, Integer, Symbol, Float, TrueClass, FalseClass, NilClass */
            if (RB_TYPE_P(literal, T_STRING)) {
                *literal_default = literal;
                return on_nil_literal_string;
            }

            if (RB_INTEGER_TYPE_P(literal) || RB_SYMBOL_P(literal) || RB_FLOAT_TYPE_P(literal) || literal == Qtrue ||
                literal == Qfalse || literal == Qnil) {
                *literal_default = literal;
                return on_nil_literal;
            }

            return on_nil_fetch_default;
        }

        /* when ApplyEmptyArrayDefault */
        VALUE ApplyEmptyArrayDefault =
            sorbet_getConstantAt(rb_cObject, rb_intern("T::Props::Private::ApplyEmptyArrayDefault"));
        if (RTEST(rb_obj_is_kind_of(default_, ApplyEmptyArrayDefault))) {
            return (nil_handler)rb_ary_new;
        }

        /* when ApplyEmptyHashDefault */
        VALUE ApplyEmptyHashDefault =
            sorbet_getConstantAt(rb_cObject, rb_intern("T::Props::Private::ApplyEmptyHashDefault"));
        if (RTEST(rb_obj_is_kind_of(default_, ApplyEmptyHashDefault))) {
            return (nil_handler)rb_hash_new;
        }

        return on_nil_fetch_default;
    }

    if (RTEST(raise_on_nil_write)) {
        return on_nil_required_prop_missing_from_deserialize;
    }

    return on_nil_nil;
}

struct deserialize_desc_closure {
    struct serde_ctx ctx;
    VALUE defaults;
    struct prop_descs_vec *descs_array;
    size_t i;
};

/* prop: Symbol
 * rules: T::Hash[Symbol, T.untyped]
 * arg: a `struct deserialize_desc_closure *` masquerading as a Ruby VALUE
 */
static int deserialize_desc_for_prop(VALUE prop, VALUE rules, VALUE arg) {
    struct deserialize_desc_closure *closure = (struct deserialize_desc_closure *)arg;
    VALUE hash_key;
    ID ivar_id;

    validate_prop(prop, rules, &hash_key, &ivar_id);

    VALUE type_object = rb_hash_fetch(rules, RB_ID2SYM(rb_intern("type_object")));
    VALUE underlying_type = rb_funcallv(rb_mT_Utils_Nilable, rb_intern("get_underlying_type_object"), 1, &type_object);

    struct transform_step *step = step_for_type(&closure->ctx, underlying_type, Deserialize);

    /* Determine the nil handler and the actual default to use. */
    VALUE default_ = rb_hash_aref(closure->defaults, prop);
    VALUE nilable_type = rb_funcallv(rb_mT_Props_Utils, rb_intern("optional_prop?"), 1, &rules);
    VALUE raise = rb_hash_aref(rules, RB_ID2SYM(rb_intern("raise_on_nil_write")));
    /* Modified if we have a literal default, value doesn't matter otherwise.  */
    VALUE literal_default = Qnil;
    nil_handler handler = generate_nil_handler(default_, nilable_type, raise, &literal_default);

    /* value of this doesn't matter for deserialization */
    bool fully_optional = false;
    struct prop_desc *desc = &closure->descs_array->descs[closure->i++];
    desc_new(desc, ivar_id, fully_optional, prop, literal_default, hash_key, step, handler);

    return ST_CONTINUE;
}

static VALUE props_reject_block(VALUE block_arg, VALUE data, int argc, const VALUE *argv, VALUE block_as_proc) {
    return rb_hash_aref(argv[1], RB_ID2SYM(rb_intern("dont_store")));
}

static struct prop_descs_vec *prop_descs_vec_alloc(size_t length) {
    struct prop_descs_vec *vec = calloc(sizeof(*vec), 1);
    vec->n_descs = length;
    vec->descs = calloc(sizeof(*vec->descs), length);
    return vec;
}

/* We use functions for these to reduce duplication between the generation bits and
 * the bits that handle serialization/deserialization, since we need the identifiers
 * to stay consistent between the two.  We could have used `static const char[]`
 * variables, but then we wouldn't benefit from the optimization in include/ruby/ruby.h
 * for rb_intern with a constant character array literal argument.
 */
static inline ID serialize_descs_id() {
    return rb_intern("@sorbet_serialize_descs");
}
static inline ID deserialize_descs_id() {
    return rb_intern("@sorbet_deserialize_descs");
}
static inline ID serialize_method_id() {
    return rb_intern("__t_props_generated_serialize");
}
static inline ID deserialize_method_id() {
    return rb_intern("__t_props_generated_deserialize");
}

/* self: Class
 * props: T::Hash[Symbol, T::Hash[Symbol, T.untyped]]
 */
static struct prop_descs_vec *generate_serialize_descs(VALUE self, VALUE props) {
    VALUE stored_props = rb_block_call(props, rb_intern("reject"), 0, NULL, props_reject_block, Qnil);
    size_t n_props = rb_hash_size_num(stored_props);
    struct prop_descs_vec *descs = prop_descs_vec_alloc(n_props);

    struct serialize_descs_closure closure;
    closure.ctx.module_index = 0;
    closure.ctx.module_array = rb_ary_new();
    closure.ctx.call_cache_index = 0;
    closure.ctx.call_cache_array = rb_ary_new();
    closure.descs_array = descs;
    closure.i = 0;

    rb_hash_foreach(stored_props, serialize_desc_for_prop, (VALUE)&closure);

    transfer_context(&closure.ctx, descs);

    return descs;
}

/* self: the object being serialized
 * strict: T::Boolean
 *
 * returns: T::Hash[String, T.untyped]
 */
static VALUE t_props_generated_serialize_impl(VALUE self, VALUE strict) {
    VALUE descs_value = rb_ivar_get(rb_obj_class(self), serialize_descs_id());
    struct prop_descs_vec *vec;
    TypedData_Get_Struct(descs_value, struct prop_descs_vec, &prop_descs_vec_type, vec);
    long descs_length = vec->n_descs;
    VALUE h = rb_hash_new_with_size(descs_length);

    struct transform_ctx ctx;
    ctx.n_modules = vec->n_modules;
    ctx.modules = vec->modules;
    ctx.caches = vec->caches;
    ctx.strict = strict;

    for (long i = 0; i < descs_length; ++i) {
        struct prop_desc *desc = &vec->descs[i];
        VALUE ivar = sorbet_vm_getivar(self, desc->ivar_id, &desc->iv_cache);
        VALUE nil_p = rb_funcallv(ivar, idNilP, 0, NULL);

        /* https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/private/serializer_generator.rb#L47-L63
         */
        if (nil_p) {
            if (RTEST(strict) && !desc->fully_optional) {
                rb_funcallv(self, rb_intern("required_prop_missing_from_serialize"), 1, &desc->prop);
            }
            /* Don't serialize values that are nil to save space (both the nil
             * value itself and the field name in the serialized BSON document).
             */
        } else {
            struct transform_step *step = desc->transform;
            rb_hash_aset(h, desc->hash_key, step->fun(ivar, &ctx, step));
        }
    }

    return h;
}

/* self: Class
 * props: T::Hash[Symbol, T::Hash[Symbol, T.untyped]]
 * defaults: T::Hash[Symbol, T::Props::Private::ApplyDefault]
 */
static struct prop_descs_vec *generate_deserialize_descs(VALUE self, VALUE props, VALUE defaults) {
    VALUE stored_props = rb_block_call(props, rb_intern("reject"), 0, NULL, props_reject_block, Qnil);
    size_t n_props = rb_hash_size_num(stored_props);
    struct prop_descs_vec *descs = prop_descs_vec_alloc(n_props);

    struct deserialize_desc_closure closure;
    closure.ctx.module_index = 0;
    closure.ctx.module_array = rb_ary_new();
    closure.ctx.call_cache_index = 0;
    closure.ctx.call_cache_array = rb_ary_new();
    closure.defaults = defaults;
    closure.descs_array = descs;
    closure.i = 0;

    rb_hash_foreach(stored_props, deserialize_desc_for_prop, (VALUE)&closure);

    transfer_context(&closure.ctx, descs);

    return descs;
}

struct deserialize_step_closure {
    VALUE val;
    struct transform_ctx *ctx;
    struct transform_step *step;
};

static VALUE run_deserialize_step(VALUE closure) {
    struct deserialize_step_closure *c = (struct deserialize_step_closure *)closure;
    return c->step->fun(c->val, c->ctx, c->step);
}

static VALUE catch_nomethod_error(VALUE x, VALUE exception) {
    VALUE *errptr = (VALUE *)x;
    *errptr = exception;
    return Qundef;
}

/* Represents the state of a prop in the hash for deserialization.  */
enum prop_existence {
    NotPresent,
    WasNil,
    Present,
};

/* hash: Hash, not a subtype */
static enum prop_existence retrieve_prop_from_hash_fast(VALUE hash, struct prop_desc *desc, VALUE *out) {
    /* Use Qundef so we don't have to do a double lookup in the case of a
     * not-present hash_key; we can also avoid a nil? send.
     * cf.
     * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/private/deserializer_generator.rb#L85-L93
     */
    VALUE val = rb_hash_lookup2(hash, desc->hash_key, Qundef);
    *out = val;

    if (val == Qundef) {
        return NotPresent;
    }

    if (val == Qnil) {
        return WasNil;
    }

    return Present;
}

/* hash: Hash, not a subtype, known to be !RHASH_AR_TABLE_P */
static enum prop_existence retrieve_prop_from_hash_st_fast(VALUE hash, struct prop_desc *desc, VALUE *out) {
    /* Use Qundef so we don't have to do a double lookup in the case of a
     * not-present hash_key; we can also avoid a nil? send.
     * cf.
     * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/private/deserializer_generator.rb#L85-L93
     */
    VALUE val;
    st_data_t stored;
    if (st_lookup_with_precomputed_hash(RHASH_ST_TABLE(hash), desc->hash_key, &stored, desc->precomputed_hash)) {
        val = (VALUE)stored;
    } else {
        val = Qundef;
    }
    *out = val;

    if (val == Qundef) {
        return NotPresent;
    }

    if (UNLIKELY(val == Qnil)) {
        return WasNil;
    }

    return Present;
}

/* hash: a proper subtype of Hash or some other object responding to Hash's methods */
static enum prop_existence retrieve_prop_from_hash_slow(VALUE hash, struct prop_desc *desc, VALUE *out) {
    /* Some Stripe code passes in custom hashes to deserialization that rely on
     * having their #[] method called.  So we have to explicitly call that here,
     * which means we also have to be somewhat slower in case we didn't find the
     * prop in the hash.
     */
    VALUE val = rb_funcallv(hash, rb_intern("[]"), 1, &desc->hash_key);
    *out = val;

    if (val == Qnil) {
        VALUE has_key_p = rb_funcallv(hash, rb_intern("key?"), 1, &desc->hash_key);
        return RTEST(has_key_p) ? WasNil : NotPresent;
    }

    return Present;
}

static __attribute__((always_inline)) VALUE
run_deserialization(VALUE self, VALUE hash, struct prop_descs_vec *vec,
                    enum prop_existence (*retrieval_func)(VALUE, struct prop_desc *, VALUE *)) {
    long descs_length = vec->n_descs;
    long found = descs_length;

    struct transform_ctx ctx;
    ctx.n_modules = vec->n_modules;
    ctx.modules = vec->modules;
    ctx.caches = vec->caches;
    ctx.strict = Qfalse;

    for (long i = 0; i < descs_length; ++i) {
        struct prop_desc *desc = &vec->descs[i];
        VALUE val;
        enum prop_existence status = retrieval_func(hash, desc, &val);
        switch (status) {
            case NotPresent:
                found -= 1;
                /* fallthrough */
            case WasNil:
                val = desc->on_nil(self, desc);
                break;
            case Present: {
                struct transform_step *step = desc->transform;
                /* avoid begin/rescue overhead if we don't have to do anything
                 * cf.
                 * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/private/deserializer_generator.rb#L51-L75
                 */
                if (!step_is_passthrough(step)) {
                    struct deserialize_step_closure closure;
                    closure.val = val;
                    closure.ctx = &ctx;
                    closure.step = step;
                    VALUE err = Qundef;

                    VALUE result = rb_rescue2(run_deserialize_step, (VALUE)&closure, catch_nomethod_error, (VALUE)&err,
                                              rb_eNoMethodError, 0);
                    if (result == Qundef) {
                        rb_funcall(self, rb_intern("raise_deserialization_error"), 3, desc->prop, val, err);
                        /* Leave val alone in case the error handler returns */
                    } else {
                        val = result;
                    }
                }
                break;
            }
        }

        sorbet_vm_setivar(self, desc->ivar_id, val, &desc->iv_cache);
    }

    return LONG2FIX(found);
}

static VALUE t_props_generated_deserialize_impl(VALUE self, VALUE hash) {
    VALUE descs_value = rb_ivar_get(rb_obj_class(self), deserialize_descs_id());
    struct prop_descs_vec *vec;
    TypedData_Get_Struct(descs_value, struct prop_descs_vec, &prop_descs_vec_type, vec);

    /* TODO: maybe we should look at HASH_REDEFINED_OP_FLAG too? */
    if (LIKELY(can_directly_call_hash_functions(hash))) {
        if (RHASH_AR_TABLE_P(hash)) {
            return run_deserialization(self, hash, vec, retrieve_prop_from_hash_fast);
        }

        return run_deserialization(self, hash, vec, retrieve_prop_from_hash_st_fast);
    }

    return run_deserialization(self, hash, vec, retrieve_prop_from_hash_slow);
}

/* We do a little dance to be compatible with sorbet-runtime.  When props are added
 * to serializable things, a lazy method definition for each of
 * __t_props_generated_serialize and __t_props_generated_deserialize gets enqueued:
 *
 * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/serializable.rb#L200-L207
 *
 * Lazy definition enqueueing is defined here:
 *
 * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/has_lazily_specialized_methods.rb#L64-L77
 *
 * The method is defined, on first call, to evaluate the string returned by the block
 * in add_prop_definition, above.
 *
 * https://github.com/sorbet/sorbet/blob/a3dd6e828748c0a8d44cb4995ca72233bbcea837/gems/sorbet-runtime/lib/types/props/has_lazily_specialized_methods.rb#L51-L62
 *
 * However, we don't have a source string that we can provide in any reasonable sense.
 * We could define a secret method and then return a call to that method, letting the
 * called secret method set up everything.  But that seems like a bit too much indirection.
 *
 * Instead, we're going to put a configuration hook in sorbet-runtime for indicating
 * whether to define serialization and deserialization methods via Ruby hooks (the
 * status quo), or whether to define serialization and deserialization via these
 * secret methods in the compiler.  We can choose between these via a configuration
 * option in sorbet-runtime.
 *
 * We define:
 *
 * T::Props::Private::SerializerGenerator.generate2(klass, props)
 * T::Props::Private::DeserializerGenerator.generate2(klass, props, defaults)
 *
 * which point at these methods.  If the appropriate configuration option is set, the
 * lazy deserialization hooks will call the above methods, which will then install the
 * appropriate __t_props_generated_{serialize,deserialize} method on klass.
 */
VALUE T_Props_Private_Serialize_generate2(VALUE recv, VALUE klass, VALUE props) {
    struct prop_descs_vec *vec = generate_serialize_descs(klass, props);

    rb_ivar_set(klass, serialize_descs_id(), TypedData_Wrap_Struct(0, &prop_descs_vec_type, vec));

    ID method = serialize_method_id();
    rb_define_method_id(klass, method, t_props_generated_serialize_impl, 1);
    return RB_ID2SYM(method);
}

VALUE T_Props_Private_Deserialize_generate2(VALUE self, VALUE klass, VALUE props, VALUE defaults) {
    struct prop_descs_vec *vec = generate_deserialize_descs(klass, props, defaults);

    rb_ivar_set(klass, deserialize_descs_id(), TypedData_Wrap_Struct(0, &prop_descs_vec_type, vec));

    ID method = deserialize_method_id();
    rb_define_method_id(klass, method, t_props_generated_deserialize_impl, 1);
    return RB_ID2SYM(method);
}
