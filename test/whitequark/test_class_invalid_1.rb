# typed: true

def self.a; class Foo; end; end # parser-error: class definition in method body
