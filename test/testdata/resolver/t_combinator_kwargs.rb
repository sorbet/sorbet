# typed: true

module M
  extend T::Sig

  sig do
    params(
      nilable: T.nilable(x: Integer),
      #        ^^^^^^^^^^^^^^^^^^^^^ error: `T.nilable` does not accept keyword arguments
      all: T.all(Integer, M, a: Integer, b: String),
      #    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `T.all` does not accept keyword arguments
      #                      ^ error: Unexpected bare `Symbol(:a)` value found in type position
      #                                  ^ error: Unexpected bare `Symbol(:b)` value found in type position
      any: T.any(Integer, String, a: Integer, b: String),
      #    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `T.any` does not accept keyword arguments
      #                           ^ error: Unexpected bare `Symbol(:a)` value found in type position
      #                                       ^ error: Unexpected bare `Symbol(:b)` value found in type position
      param: T.type_parameter(a: Integer),
      #      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `T.type_parameter` does not accept keyword arguments
      enum: T.deprecated_enum(a: Integer),
      #                       ^^^^^^^^^^ error: Expected `T.any(T::Set[T.anything], T::Array[T.anything])`
      #     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `T.deprecated_enum` does not accept keyword arguments
      klass: T.class_of(a: Integer),
      #      ^^^^^^^^^^^^^^^^^^^^^^ error: `T.class_of` does not accept keyword arguments
    ).void
  end
  def test(nilable, all, any, param, enum, klass)
  end

end
