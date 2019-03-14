# typed: true
def a(arg1, arg2); end;
def b(arg1); end;
def c(arg1); end;
def d; end;
a b(c d), 1.0r do end
