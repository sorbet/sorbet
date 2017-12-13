# @typed

class TestSubtype
  def test_subtype(a)
    a = Opus::Types.cast(nil, Opus::Types.enum([:foo]))
    Opus::Types.assert_type!(a, Opus::Types.enum([:foo, :bar]))
  end
end
