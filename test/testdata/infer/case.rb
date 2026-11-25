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

  def test_narrowing_subclass_after_lambda
    x = T.let("", Object)

    case x
    when ->(y) { false }
    when String
      # Narrows correctly even with the `-> (y) { false }` lambda above.
      T.reveal_type(x)
    # ^^^^^^^^^^^^^^^^ error: Revealed type: `String`
    end
  end

  def test_narrowing_union_after_lambda
    x = T.let(123, T.any(Integer, String))

    case x
    when ->(y) { false }
    when String
    when Integer
      # Narrows correctly even with the `-> (y) { false }` lambda above.
      T.reveal_type(x)
    # ^^^^^^^^^^^^^^^^ error: Revealed type: `Integer`
    else T.absurd(x)
    end
  end
end
