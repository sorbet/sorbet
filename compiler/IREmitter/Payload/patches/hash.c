typedef VALUE (*BlockFFIType)(VALUE firstYieldedArg, VALUE closure, int argCount, const VALUE *args, VALUE blockArg);

// A faster version of merge!, under the assumption that we don't pass blocks, and only pass a single arg.
VALUE sorbet_magic_mergeHashHelper(VALUE self, VALUE hash) {
    rb_hash_foreach(hash, rb_hash_update_i, self);
    return self;
}

/* expose any_hash so we can precompute hash values for strings */
long sorbet_hash_string(VALUE a) {
    /* we should not need other_func, so pass NULL */
    return any_hash(a, NULL);
}

// The no-block version of rb_hash_any_p, with any logic conditional on a block
// being present removed.
VALUE sorbet_rb_hash_any_forwarder(int argc, VALUE *argv, VALUE hash) {
    VALUE args[2];
    args[0] = Qfalse;

    rb_check_arity(argc, 0, 1);
    if (RHASH_EMPTY_P(hash)) {
        return Qfalse;
    }

    if (argc) {
        args[1] = argv[0];
        rb_hash_foreach(hash, any_p_i_pattern, (VALUE)args);
        return args[0];
    } else {
        /* yields pairs, never false */
        return Qtrue;
    }
}

// Avoid dispatch through the VM by calling the body of `rb_hash_update` directly.
void sorbet_hashUpdate(VALUE hash, VALUE other) {
    rb_hash_update(1, &other, hash);
}

VALUE (*sorbet_rb_hash_to_h_func(void))(VALUE) {
    return rb_hash_to_h;
}

// The no-block version of `rb_hash_to_h`
VALUE sorbet_rb_hash_to_h(VALUE hash) {
    if (rb_obj_class(hash) != rb_cHash) {
        const VALUE flags = RBASIC(hash)->flags;
        hash = hash_dup(hash, rb_cHash, flags & RHASH_PROC_DEFAULT);
    }
    return hash;
}

// no-block rb_hash_fetch_m: https://github.com/ruby/ruby/blob/ruby_2_7/hash.c#L2113-L2147
VALUE sorbet_rb_hash_fetch_m(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                             VALUE closure) {
    VALUE key;
    st_data_t val;

    rb_check_arity(argc, 1, 2);
    key = argv[0];

    if (hash_stlike_lookup(recv, key, &val)) {
        return (VALUE)val;
    } else {
        if (argc == 1) {
            VALUE desc = rb_protect(rb_inspect, key, 0);
            if (NIL_P(desc)) {
                desc = rb_any_to_s(key);
            }
            desc = rb_str_ellipsize(desc, 65);
            rb_key_err_raise(rb_sprintf("key not found: %" PRIsVALUE, desc), recv, key);
        } else {
            return argv[1];
        }
    }
}
