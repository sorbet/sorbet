# typed: true

def foo(x); end
def foo(x = 1); end # error: redefined with argument `x` as an optional argument

foo()
foo(0)
