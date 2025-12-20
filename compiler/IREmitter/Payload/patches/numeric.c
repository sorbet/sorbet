
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
        // fall through to coerce
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_minus(recv, y);
    }
    return rb_num_coerce_bin(recv, y, '-');
}

VALUE sorbet_rb_int_gt_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        return fix_gt(recv, y);
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_gt(recv, y);
    }
    return rb_num_coerce_relop(recv, y, '>');
}

VALUE sorbet_rb_int_lt_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        return fix_lt(recv, y);
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_lt(recv, y);
    }
    return rb_num_coerce_relop(recv, y, '<');
}

VALUE sorbet_rb_int_ge_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        return fix_ge(recv, y);
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_ge(recv, y);
    }
    return rb_num_coerce_relop(recv, y, idGE);
}

VALUE sorbet_rb_int_le_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        return fix_le(recv, y);
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_le(recv, y);
    }
    return rb_num_coerce_relop(recv, y, idLE);
}
