# typed: true

def m(foo = def foo.m; end); end # parser-error: circular argument reference foo
