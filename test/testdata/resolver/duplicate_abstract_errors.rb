# typed: true
class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(Integer)}
  def self.foo; end
end

class Child < Parent # error: Missing definition for abstract method `Parent.foo`
  class << self
    extend T::Sig
  end
end
