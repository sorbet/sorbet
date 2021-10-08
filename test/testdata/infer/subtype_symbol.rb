# typed: true

class TestSubtype
  def test_subtype(a)
    b = T.cast(nil, T.deprecated_enum([:foo]))
    T.assert_type!(b, T.deprecated_enum([:foo, :bar]))
  end
end
