# typed: true

class Foo
  extend T::Sig
  sig { returns(String) }
  def foo; 1; end # error: Expected `String` but found `Integer(1)` for method result type
end
