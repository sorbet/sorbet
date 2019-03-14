# typed: true
def fun; end;
def foo; end;
def bar; end;
fun(foo, *bar) # error: Splats are only supported where the size of the array is known statically
