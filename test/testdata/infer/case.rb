# @typed
class TestCase
  def f(x)
    case x
    when Integer
      T.assert_type!(x, Integer)
    when Array, Hash
      T.assert_type!(x, T.any(Array, Hash))
    end
  end
end
