# typed: true

class A
  extend T::Sig
  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U))
      .void
  end
  def f1(x)
    T.reveal_type(x) # error: `T.type_parameter(:U) (of A#f1)`

    type = T::Array[x]
    #               ^ error: Unexpected bare `T.type_parameter(:U) (of A#f1)` value found in type position
    T.reveal_type(type) # error: `Runtime object representing type: T::Array[T.untyped]`

    type = T.class_of(x)
    #                 ^ error: Unexpected bare `T.type_parameter(:U) (of A#f1)` value found in type position
    T.reveal_type(type) # error: `Runtime object representing type: T.untyped`

    type = T.any(x, x)
    #            ^ error: Unexpected bare `T.type_parameter(:U) (of A#f1)` value found in type position
    #               ^ error: Unexpected bare `T.type_parameter(:U) (of A#f1)` value found in type position
    T.reveal_type(type) # error: `Runtime object representing type: T.untyped`

    type = T.all(x, x)
    #            ^ error: Unexpected bare `T.type_parameter(:U) (of A#f1)` value found in type position
    #               ^ error: Unexpected bare `T.type_parameter(:U) (of A#f1)` value found in type position
    T.reveal_type(type) # error: `Runtime object representing type: T.untyped`

    type = T.nilable(x)
    #                ^ error: Unexpected bare `T.type_parameter(:U) (of A#f1)` value found in type position
    T.reveal_type(type) # error: `Runtime object representing type: T.untyped`

    type = T.proc.params(arg0: x).void
    #                          ^ error: Unexpected bare `T.type_parameter(:U) (of A#f1)` value found in type position
    T.reveal_type(type) # error: `Runtime object representing type: T.proc.params(arg0: T.untyped).void`

    type = T.proc.returns(x)
    #                     ^ error: Unexpected bare `T.type_parameter(:U) (of A#f1)` value found in type position
    T.reveal_type(type) # error: `Runtime object representing type: T.proc.returns(T.untyped)`
  end

  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U))
      .void
  end
  def f2(x)
    T.reveal_type(x) # error: `T.type_parameter(:U) (of A#f2)`
    f = T.let(
      -> (x) {
        T.reveal_type(x) # error: `T.untyped`
      },
      T.proc.params(arg0: x).void
      #                   ^ error: Unsupported type syntax
      #                   ^ error: Unexpected bare `T.type_parameter(:U) (of A#f2)` value found in type position
    )
    T.reveal_type(f) # error: `T.proc.params(arg0: T.untyped).void`
  end

  sig do
    type_parameters(:U)
      .params(type: T::Types::Base)
      .void
  end
  def f3(type)
    T.nilable(type)
    #         ^^^^ error: Unexpected bare `T::Types::Base` value found in type position
    T.nilable(T.unsafe(type))
  end
end

