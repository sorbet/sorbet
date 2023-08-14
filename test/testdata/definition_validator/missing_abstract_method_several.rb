# typed: true
# selective-apply-code-action: quickfix
class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.void }
  def foo; end

  sig { abstract.params(baz: Integer).returns(String) }
  private def bar(baz); end

  sig { abstract.returns(Integer) }
  protected def qux; end
end

  class Child < Parent
# ^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method `Parent#foo`
# ^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method `Parent#bar`
# ^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method `Parent#qux`
  end
