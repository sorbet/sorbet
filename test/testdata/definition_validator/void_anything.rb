# typed: true

class Parent
  extend T::Sig, T::Helpers
  abstract!

  sig { abstract.void }
  def example1; end

  sig { abstract.returns(T.anything) }
  def example2; end

  sig { abstract.returns(Integer) }
  def example3; end
end

class Child < Parent
  sig { override.returns(T.anything) }
  def example1; end

  sig { override.void }
  def example2; end

  sig { override.void }
  def example3; end # error: Return type `void` does not match return type of abstract method `Parent#example3`
end
