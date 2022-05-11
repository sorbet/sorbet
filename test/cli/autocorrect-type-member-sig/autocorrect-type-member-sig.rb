# typed: strict

class Parent
  extend T::Sig
  extend T::Generic

  X = type_member

  sig {params(x: X).void}
  def foo(x); end
end

class Child < Parent
  X = type_member
  def foo(x); end
end
