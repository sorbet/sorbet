# typed: true

class Parent
  extend T::Sig, T::Helpers
  abstract!
  sig { abstract.params(x: T.class_of(Numeric)[Integer]).void }
  def foo(x)
  end
end

class Child < Parent; end # error: Missing definition for abstract method `Parent#foo`
