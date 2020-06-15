# typed: true

def m(foo = class << foo; end) end # error: circular argument reference foo
