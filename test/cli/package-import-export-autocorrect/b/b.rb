# typed: strict

module B
  extend T::Sig

  class Bar; end

  sig {returns(A::Foo)}
  def use_a
    T.unsafe(nil)
  end
end
