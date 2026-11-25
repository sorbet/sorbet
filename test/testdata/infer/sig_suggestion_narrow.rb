# typed: strict

class Parent
  extend T::Sig

  sig { returns(T.any(Integer, String)) }
  def foo = 0
end

class Child < Parent
  def foo = 0 # error: does not have a `sig`
end
