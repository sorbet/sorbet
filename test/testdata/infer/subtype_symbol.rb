# typed: true

class TestSubtype
  def test_subtype(a)
    b = T.cast(nil, T.deprecated_enum([:foo]))
    T.assert_type!(b, T.deprecated_enum([:foo, :bar]))
  end
end

class GenericWithDeprecatedEnum
  extend T::Sig, T::Generic
  Elem = type_member

  # used to cause a crash
  sig { params(x: T.deprecated_enum([3])).void }
  def example(x)
  end
end
