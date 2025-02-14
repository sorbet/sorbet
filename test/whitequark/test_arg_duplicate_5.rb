# typed: true

def foo(aa, *r, aa); end # parser-error: duplicate argument name
