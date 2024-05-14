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
    T.reveal_type(type) # error: `Runtime object representing type: T::Array[T.type_parameter(:U) (of A#f1)]`

    type = T.class_of(x)
    T.reveal_type(type) # error: `T.untyped`

    type = T.any(x, x)
    T.reveal_type(type) # error: `Runtime object representing type: T.type_parameter(:U) (of A#f1)`

    type = T.all(x, x)
    T.reveal_type(type) # error: `Runtime object representing type: T.type_parameter(:U) (of A#f1)`

    type = T.nilable(x)
    T.reveal_type(type) # error: `Runtime object representing type: T.nilable(T.type_parameter(:U) (of A#f1))`

    type = T.proc.params(arg0: x).void
    T.reveal_type(type) # error: `Runtime object representing type: T.proc.params(arg0: T.type_parameter(:U) (of A#f1)).void`

    type = T.proc.returns(x)
    T.reveal_type(type) # error: `Runtime object representing type: T.proc.returns(T.type_parameter(:U) (of A#f1))`
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
        T.reveal_type(x) # error: `T.type_parameter(:U) (of A#f2)`
      },
      T.proc.params(arg0: x).void
      #                   ^ error: Unsupported type syntax
    )
    T.reveal_type(f) # error: `T.proc.params(arg0: T.untyped).void`
  end
end
