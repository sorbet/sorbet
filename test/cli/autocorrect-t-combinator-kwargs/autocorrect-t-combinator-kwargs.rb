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
    ).void
  end
  def test(nilable, all, any, param, enum, klass)
  end

end
