
typedef VALUE (*BlockFFIType)(VALUE firstYieldedArg, VALUE closure, int argCount, const VALUE *args, VALUE blockArg);

VALUE sorbet_rb_array_square_br_slowpath(VALUE recv, ID fun, int argc, const VALUE *const restrict argv,
                                         BlockFFIType blk, VALUE closure) {
    VALUE ary = recv;
    // Don't call rb_ary_aref directly because we already checked the arity.
    if (argc == 2) {
        return rb_ary_aref2(ary, argv[0], argv[1]);
    }
    // This is slightly inefficient, as we arrived in this function because we already
    // handled the argc == 1 && FIXNUM_P(argv[0]) case.  But it's less code duplication.
    return rb_ary_aref1(ary, argv[0]);
}

VALUE rb_ary_compact_bang_forwarder(VALUE ary) {
    return rb_ary_compact_bang(ary);
}

VALUE sorbet_ary_make_hash(VALUE ary) {
    return ary_make_hash(ary);
}

void sorbet_ary_recycle_hash(VALUE hash) {
    ary_recycle_hash(hash);
}

// This is the no-block version of rb_ary_uniq: https://github.com/ruby/ruby/blob/ruby_2_7/array.c#L5018-L5041
VALUE sorbet_rb_array_uniq(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                           VALUE closure) {
    rb_check_arity(argc, 0, 0);
    VALUE ary = recv;

    VALUE hash, uniq;

    if (RARRAY_LEN(ary) <= 1) {
        hash = 0;
        uniq = rb_ary_dup(ary);
    } else {
        hash = ary_make_hash(ary);
        uniq = rb_hash_values(hash);
    }
    RBASIC_SET_CLASS(uniq, rb_obj_class(ary));
    if (hash) {
        ary_recycle_hash(hash);
    }

    return uniq;
}

VALUE (*sorbet_rb_ary_to_a_func(void))(VALUE) {
    return rb_ary_to_a;
}

VALUE sorbet_rb_ary_to_a(VALUE ary) {
    return rb_ary_to_a(ary);
}

void sorbet_rb_ary_set_len(VALUE ary, long n) {
    ARY_SET_LEN(ary, n);
}
