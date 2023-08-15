# typed: true

class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.params(bar: Integer).returns(String)}
  def foo(bar); end
end

  class Child < Parent
# ^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method
  end
