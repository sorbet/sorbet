
typedef VALUE (*BlockFFIType)(VALUE firstYieldedArg, VALUE closure, int argCount, const VALUE *args, VALUE blockArg);

// Trying to be a copy of rb_mod_const_get, specialized to a single id argument
VALUE sorbet_getConstantAt(VALUE mod, ID id) {
    VALUE name;
    rb_encoding *enc;
    const char *pbeg, *p, *path, *pend;
    int recur = 1;
    int DISABLED_CODE = 0;

    name = rb_id2str(id);
    enc = rb_enc_get(name);
    path = rb_id2name(id);

    pbeg = p = path;
    pend = path + strlen(path);

    if (DISABLED_CODE && (p >= pend || !*p)) {
    wrong_name:
        rb_raise(rb_eRuntimeError, "wrong constant name %" PRIsVALUE "%" PRIsVALUE, mod, name);
    }

    if (DISABLED_CODE && (p + 2 < pend && p[0] == ':' && p[1] == ':')) {
        mod = rb_cObject;
        p += 2;
        pbeg = p;
    }

    while (p < pend) {
        VALUE part;
        long len, beglen;

        while (p < pend && *p != ':')
            p++;

        if (pbeg == p)
            goto wrong_name;

        id = rb_check_id_cstr(pbeg, len = p - pbeg, enc);
        beglen = pbeg - path;

        if (p < pend && p[0] == ':') {
            if (p + 2 >= pend || p[1] != ':')
                goto wrong_name;
            p += 2;
            pbeg = p;
        }

        if (!RB_TYPE_P(mod, T_MODULE) && !RB_TYPE_P(mod, T_CLASS)) {
            rb_raise(rb_eTypeError, "%" PRIsVALUE " does not refer to class/module", name);
        }

        if (!id) {
            part = rb_str_subseq(name, beglen, len);
            OBJ_FREEZE(part);
            VALUE idConst_missing = rb_intern("const_missing");
            if (!rb_is_const_name(part)) {
                name = part;
                goto wrong_name;
            } else if (!rb_method_basic_definition_p(CLASS_OF(mod), idConst_missing)) {
                part = rb_str_intern(part);
                mod = rb_const_missing(mod, part);
                continue;
            } else {
                rb_mod_const_missing(mod, part);
            }
        }
        if (!rb_is_const_id(id)) {
            name = ID2SYM(id);
            goto wrong_name;
        }
        if (!recur) {
            mod = rb_const_get_at(mod, id);
        } else if (beglen == 0) {
            mod = rb_const_get(mod, id);
        } else {
            mod = rb_const_get_from(mod, id);
        }
    }

    return mod;
}

VALUE sorbet_getConstant(const char *path, long pathLen) {
    ID id = rb_intern2(path, pathLen);
    return sorbet_getConstantAt(rb_cObject, id);
}

void sorbet_setConstant(VALUE mod, const char *name, long nameLen, VALUE value) {
    ID id = rb_intern2(name, nameLen);
    return rb_const_set(mod, id, value);
}

// This doesn't do exactly the right thing because that is done by the parser in Ruby. Ruby will return the String
// "expression" if the RHS is an expression.
VALUE sorbet_definedIntrinsic(VALUE recv, ID fun, int argc, const VALUE *const restrict argv, BlockFFIType blk,
                              VALUE closure) {
    if (argc == 0) {
        return RUBY_Qnil;
    }
    VALUE klass = rb_cObject;
    for (int i = 0; i < argc; i++) {
        VALUE str = argv[i];
        ID id = rb_intern(RSTRING_PTR(str));
        if (!rb_const_defined_at(klass, id)) {
            return RUBY_Qnil;
        }
        klass = sorbet_getConstantAt(klass, id);
    }
    return rb_str_new2("constant");
}

VALUE sorbet_vm_class_alloc(VALUE klass) {
    return rb_class_alloc(klass);
}

VALUE (*sorbet_vm_Class_new_func(void))() {
    return rb_class_s_new;
}

VALUE (*sorbet_vm_Kernel_instance_variable_get_func(void))(VALUE obj, VALUE iv) {
    return rb_obj_ivar_get;
}

VALUE (*sorbet_vm_Kernel_instance_variable_set_func(void))(VALUE obj, VALUE iv, VALUE value) {
    return rb_obj_ivar_set;
}
