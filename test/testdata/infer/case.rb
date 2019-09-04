# typed: true
class TestCase
  def f(x)
    case x
    when Integer
      T.assert_type!(x, Integer)
    when Array, Hash
      T.assert_type!(x, T.any(T::Array[T.untyped], T::Hash[T.untyped, T.untyped]))
    end
  end
end
