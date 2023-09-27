# typed: true

class Parent
  extend T::Sig

  sig {overridable.void}
  def foo; end
  #   ^ find-implementation: foo
end

class Child < Parent
  sig {override.void}
  def foo; end
  #   ^ implementation: foo
end
