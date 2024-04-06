# typed: strict

module B
  extend T::Sig

  sig {returns(A::Foo)}
  def use_a
    x = A::Foo.new
    T.unsafe(x)
  end
end
