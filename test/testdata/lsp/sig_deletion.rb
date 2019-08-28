# typed: true

class Foo
  extend T::Sig
  sig { returns(String) }
  def foo; 1; end # error: Returning value that does not conform to method result type
end
