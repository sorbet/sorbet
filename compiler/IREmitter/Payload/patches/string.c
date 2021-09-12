
// cf. rb_fstring_new, but always as utf-8
VALUE sorbet_vm_fstring_new(const char *ptr, long len) {
    struct RString fake_str;
    return register_fstring(setup_fake_str(&fake_str, ptr, len, ENCINDEX_UTF_8));
}
