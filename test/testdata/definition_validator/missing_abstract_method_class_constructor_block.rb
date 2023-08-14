# typed: true
# selective-apply-code-action: quickfix
class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo; end
end

  Child = Class.new(Parent) do
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method `Parent#foo`
    def bar; end
  end
