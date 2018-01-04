# @typed
class TestCase
  def f(x)
    case x
    when Integer
      Opus::Types.assert_type!(x, Integer)
    when Array, Hash
      Opus::Types.assert_type!(x, Opus::Types.any(Array, Hash))
    end
  end
end
