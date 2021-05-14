
// A faster version of merge!, under the assumption that we don't pass blocks, and only pass a single arg.
VALUE sorbet_magic_mergeHashHelper(self, hash) {
    rb_hash_foreach(hash, rb_hash_update_i, self);
    return self;
}
