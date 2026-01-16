# typed: true

class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo; end

  sig {abstract.void}
  def bar; end

  sig {abstract.void}
  def baz; end
end

  class Child1 < Parent
# ^^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method
    sig {override.void}
    def foo; end

    sig {override.void}
    def baz; end
  end

  class Child2 < Parent
# ^^^^^^^^^^^^^^^^^^^^^ error: Missing definitions for abstract methods
    sig {override.void}
    def foo; end
  end
