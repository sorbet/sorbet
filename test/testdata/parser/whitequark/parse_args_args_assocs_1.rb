# typed: true
def baz; end;
def fun(a, b); end;
def foo; end;
fun(foo, :foo => 1, &baz)
