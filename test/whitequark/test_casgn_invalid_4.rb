# typed: true

def self.f; Foo::Bar = 1; end # error: dynamic constant assignment
