# typed: true
def a(arg1, arg2); end;
def b(arg); end;
def c(arg); end;
def d; end;
a b(c(d)), 1.0 do end
