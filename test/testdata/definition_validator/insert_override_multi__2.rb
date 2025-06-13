# typed: true

class Parent
  sig { overridable.void }
  def example; end
end

class Child < Parent
  sig { void }
  def example; end # error: Method `Child#example` overrides an overridable method `Parent#example` but is not declared with `override.`
end

class Module
  include T::Sig
end
