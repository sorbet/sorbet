# typed: true

def f; Foo::Bar = 1; end # parser-error: dynamic constant assignment
