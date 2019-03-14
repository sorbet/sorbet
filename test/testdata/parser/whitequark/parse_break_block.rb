# typed: true
def foo; end;
def fun(a); end;
break fun foo do end # error: No `do` block around `break`
