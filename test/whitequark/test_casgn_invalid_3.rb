# typed: true

def self.f; Foo = 1; end # parser-error: dynamic constant assignment
