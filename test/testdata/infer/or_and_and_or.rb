# typed: strict
module Fake
end
class Test
  extend T::Helpers

  sig.returns(T.untyped)
  def untyped
  end

  sig.returns(T.all(T.any(Integer, Float), Fake))
  def and_or
    untyped
  end

  sig.returns(T.any(T.all(Integer, Fake), T.all(Float, Fake)))
  def or_and
    untyped
  end

  def test
    T.assert_type!(or_and, T.all(T.any(Integer, Float), Fake))
    T.assert_type!(and_or, T.any(T.all(Integer, Fake), T.all(Float, Fake)))
  end
end
