static VALUE rb_cvar_get_or_undef(VALUE klass, ID id) {
    st_data_t value;
    CVAR_LOOKUP(&value, return (VALUE)value);
    return Qundef;
}

int rb_cvar_lookup(VALUE klass, ID id, VALUE *value) {
    // Compare rb_cvar_get/rb_cvar_defined.
    VALUE v = rb_cvar_get_or_undef(klass, id);
    if (v == Qundef) {
        return 0;
    }
    *value = v;
    return 1;
}
