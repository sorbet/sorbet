# @typed
class TestMatch
  def test_match(x)
    if Integer === x
      Opus::Types.assert_type!(x, Integer)
    end
  end
end
