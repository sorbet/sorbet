# typed: strict

module A::A2
  extend T::Sig

  sig {void}
  def baz
    # A.foo
    # A.new.foo
  end
end
