
// A faster version of merge!, under the assumption that we don't pass blocks, and only pass a single arg.
VALUE sorbet_magic_mergeHashHelper(self, hash) {
    rb_hash_foreach(hash, rb_hash_update_i, self);
    return self;
}

/* expose any_hash so we can precompute hash values for strings */
long sorbet_hash_string(VALUE a) {
    /* we should not need other_func, so pass NULL */
    return any_hash(a, NULL);
}

/* Our intrinsic wrapping could write essentially this function out automagically,
 * but we do fancy things for Hash#any? and blocks, so we have to do everything
 * manually.
 */
VALUE sorbet_rb_hash_any_forwarder(int argc, VALUE *argv, VALUE hash) {
    return rb_hash_any_p(argc, argv, hash);
}
