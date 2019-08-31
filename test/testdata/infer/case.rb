# typed: true
class TestCase
  def f(x)
    case x
    when Integer
      T.assert_type!(x, Integer)
    when Array, Hash
      T.reveal_type(x) # error: Revealed type: `T.any(T::Hash[<any>, <any>], T::Array[<any>])`
    end
  end
end
