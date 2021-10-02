# typed: true

class TestSubtype
  def test_subtype(a)
    a = T.cast(nil, T.enum([:foo]))
    T.assert_type!(a, T.enum([:foo, :bar]))
    b = T.cast(nil, T.deprecated_enum([:foo]))
    T.assert_type!(b, T.deprecated_enum([:foo, :bar]))
  end
end
