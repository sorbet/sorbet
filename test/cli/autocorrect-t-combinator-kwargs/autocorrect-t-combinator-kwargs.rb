# typed: true

module M
  extend T::Sig

  sig do
    params(
      nilable: T.nilable(x: Integer),
      all: T.all(Integer, M, a: Integer, b: String),
      any: T.any(Integer, String, a: Integer, b: String),
      param: T.type_parameter(a: Integer),
      enum: T.deprecated_enum(a: Integer),
      klass: T.class_of(a: Integer),
      splat: T.class_of(**TYPES),
      pos_and_splat: T.class_of(Float, **TYPES),
    ).void
  end
  def test(nilable, all, any, param, enum, klass, splat, pos_and_splat)
  end

end

# This is down here so that the locs in the snapshot didn't change
TYPES = [Integer, String]
