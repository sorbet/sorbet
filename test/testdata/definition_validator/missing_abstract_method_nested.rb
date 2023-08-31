# typed: true

class Grandparent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo; end

  sig {abstract.void}
  def bar; end
end

class Parent < Grandparent
  abstract!

  sig {override.void}
  def foo; end
end

  class Child < Parent
# ^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method
  end
