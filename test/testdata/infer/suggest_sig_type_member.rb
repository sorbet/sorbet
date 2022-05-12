# typed: strict

class Parent
  extend T::Sig
  extend T::Generic

  X = type_member
  Y = type_template

  sig {params(x: X).void}
  def foo(x); end

  sig {params(y: Y).void}
  def self.foo(y); end
end

class Child < Parent
  X = type_member
  Y = type_template
  def foo(x); end # error: does not have a `sig`
  def self.foo(y); end # error: does not have a `sig`
end
