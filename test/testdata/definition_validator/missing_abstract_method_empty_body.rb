# typed: true
# selective-apply-code-action: quickfix
class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo; end
end

  class Child < Parent; end
# ^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method
