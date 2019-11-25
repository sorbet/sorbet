#include "granita.h"
#include "ruby.h"

VALUE has_array(VALUE a1, VALUE a2) {
    VALUE values[2];
    values[0] = a1;
    values[1] = a2;
    return sorbet_callFunc(a1, 0, 2, values);
}

VALUE rb_return_nil() {
    sorbet_idIntern("stuff");
    return sorbet_rubyNil();
}

/* calling converntion that we'll use */
VALUE my_method(int argc, VALUE *argv, VALUE self) {
    VALUE arr[0];
    return sorbet_callFunc(self, sorbet_idIntern("puts"), 0, arr);
}

void Init_foobar() {
    VALUE mod = rb_define_module("DemoModule");
    rb_define_singleton_method(mod, "return_nil", rb_return_nil, 0);
    rb_define_singleton_method(mod, "my_method", my_method, -1);
}
