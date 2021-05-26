
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
