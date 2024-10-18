# typed: false

def foo(a, b = 2, *c, d:, e: 5, **f, &blk); end

def foo(*, **, &); end
