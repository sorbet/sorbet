#include <ruby.h>

char *rubyfmt_rstring_ptr(VALUE s) {
  return RSTRING_PTR(s);
}

long rubyfmt_rstring_len(VALUE s) {
  return RSTRING_LEN(s);
}

enum ruby_value_type rubyfmt_rb_type(VALUE v) {
  return rb_type(v);
}

long long rubyfmt_rb_num2ll(VALUE v) {
  return RB_NUM2LL(v);
}

long rubyfmt_rb_ary_len(VALUE v) {
  return rb_array_len(v);
}

VALUE *rubyfmt_rb_ary_ptr(VALUE v) {
  return RARRAY_PTR(v);
}

int rubyfmt_rb_nil_p(VALUE v) {
  return RB_NIL_P(v);
}
