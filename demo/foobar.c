#include "ruby.h"
#include "granita.h"

VALUE rb_return_nil() {
  return sorbet_rubyNil();
}

void Init_foobar() {
  VALUE mod = rb_define_module("DemoModule");
  rb_define_singleton_method(mod, "return_nil", rb_return_nil, 0);
}
