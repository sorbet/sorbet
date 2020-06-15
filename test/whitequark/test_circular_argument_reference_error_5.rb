# typed: true

def m(foo = def foo.m; end); end # error: circular argument reference foo
