# typed: true

def f; ::A, foo = foo; end # parser-error: dynamic constant assignment
