# typed: true

class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(T.nilable(Integer))}
  def example1; end

  sig {abstract.returns(Integer)}
  def example2; end
end

class Child < Parent
  abstract!

  sig {abstract.returns(Integer)}
  def example1; end

  sig {abstract.returns(T.nilable(Integer))}
  def example2; end # error: Return type `T.nilable(Integer)` does not match return type of abstract method `Parent#example2`
end
