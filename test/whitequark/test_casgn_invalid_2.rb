# typed: true

def f; ::Bar = 1; end # parser-error: dynamic constant assignment
