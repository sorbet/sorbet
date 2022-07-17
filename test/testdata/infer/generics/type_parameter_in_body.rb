# typed: true
extend T::Sig

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .returns(T::Array[T.type_parameter(:U)])
end
def example(x)
  xs = T::Array[T.type_parameter(:U)].new
  T.reveal_type(xs) # error: `T::Array[T.type_parameter(:U) (of Object#example)]`
  xs << x
  xs << "nope"
  #     ^^^^^^ error: Expected `T.type_parameter(:U) (of Object#example)` but found `String("nope")`

  0.times do
    T.type_parameter(:U).does_not_exist
    #                    ^^^^^^^^^^^^^^ error: Call to method `does_not_exist` on `T.type_parameter(:U) (of Object#example)` mistakes a type for a value
  end

  T.type_parameter(:DoesNotExist)
  #                ^^^^^^^^^^^^^ error: Type parameter `:DoesNotExist` does not exist on `Object#example`

  xs
end
