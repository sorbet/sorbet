# @typed
class TestMatch
  def test_match(x)
    if Integer === x
      T.assert_type!(x, Integer)
    end
  end
end
