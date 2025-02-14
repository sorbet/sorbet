# typed: true

def m(foo = class << foo; end) end # parser-error: circular argument reference foo
