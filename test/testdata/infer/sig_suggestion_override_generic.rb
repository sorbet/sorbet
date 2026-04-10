# typed: strict

class A
  extend T::Sig
  sig { void }
  def initialize; end

  def self.new # error: does not have a `sig`
  end
end

class Parent
  extend T::Sig, T::Generic
  Elem = type_member

  sig { returns(Elem) }
  def foo = raise
end
class Child1 < Parent # error: must be re-declared
  def foo = raise # error: does not have a `sig`
end
class Child2 < Parent
  Elem = type_member
  def foo = raise # error: does not have a `sig`
end
