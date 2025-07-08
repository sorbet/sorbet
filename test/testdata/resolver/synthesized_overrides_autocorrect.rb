# typed: strict

class OverridableParent
  extend T::Sig

  sig { overridable.returns(Integer) }
  def foo; 0; end
end

class Child < OverridableParent
  sig { void }
  def initialize(); @foo = T.let(0, Integer); end

  sig { returns(Integer) }
  attr_reader :foo
end
