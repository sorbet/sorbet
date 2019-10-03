#include "ruby.h"
#include "granita.h"

VALUE rb_return_nil() {
  return sorbet_rubyNil();
}

/* calling converntion that we'll use */
VALUE my_method(int argc, VALUE* argv, VALUE self){
  return self;
}

void Init_foobar() {
  VALUE mod = rb_define_module("DemoModule");
  rb_define_singleton_method(mod, "return_nil", rb_return_nil, 0);
}
