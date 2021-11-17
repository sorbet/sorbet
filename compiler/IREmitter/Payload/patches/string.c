
// cf. rb_fstring_new, but always as utf-8
VALUE sorbet_vm_fstring_new(const char *ptr, long len) {
    struct RString fake_str;
    return register_fstring(setup_fake_str(&fake_str, ptr, len, ENCINDEX_UTF_8));
}

VALUE sorbet_vm_str_uplus(VALUE str) {
    return str_uplus(str);
}

VALUE (*sorbet_rb_str_to_s_func(void))(VALUE) {
    return rb_str_to_s;
}

VALUE sorbet_rb_str_to_s(VALUE str) {
    return rb_str_to_s(str);
}
