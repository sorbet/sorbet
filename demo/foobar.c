#include "ruby.h"
#include "granita.h"

VALUE rb_return_nil() {
  return sorbet_rubyNil();
}

/* calling converntion that we'll use */
VALUE my_method(int argc, VALUE* argv, VALUE self){
  /*
   * We want to define a method like
   *
   *     def my_method man1, opt1 = true, opt2 = false, *splat, man2, **opts, &blk
  */

  VALUE man1, man2;
  VALUE opt1, opt2;
  VALUE splat;
  VALUE opts;
  VALUE blk;

  rb_scan_args(argc, argv, "12*1:&", &man1, &opt1, &opt2, &splat, &man2, &opts, &blk);
  return self;
}

void Init_foobar() {
  VALUE mod = rb_define_module("DemoModule");
  rb_define_singleton_method(mod, "return_nil", rb_return_nil, 0);
}
