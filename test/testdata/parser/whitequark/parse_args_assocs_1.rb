# typed: true
def fun(arg); end;
def baz; end;
fun(:foo => 1, &baz)
