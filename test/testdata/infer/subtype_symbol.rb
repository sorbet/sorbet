# @typed

class TestSubtype
  def test_subtype(a)
    a = T.cast(nil, T.enum([:foo]))
    T.assert_type!(a, T.enum([:foo, :bar]))
  end
end
