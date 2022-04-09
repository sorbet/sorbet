# typed: true
class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(Integer)}
  def self.foo; end
end

class Child < Parent
  class << self # error: Missing definition for abstract method `Parent.foo`
    extend T::Sig
  end
end
