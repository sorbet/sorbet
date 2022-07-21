# typed: true

extend T::Sig

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .returns(T.type_parameter(:U))
end
def example(x)
  y = T.let(x, T.type_parameter(:U))
  T.reveal_type(y) # error: `T.type_parameter(:U) (of Object#example)`

  pinned = T.let(nil, T.nilable(T.type_parameter(:U)))
  1.times do
    pinned = x
  end

  needs_pin = nil
  1.times do
    needs_pin = x
    #           ^ error: Changing the type of a variable in a loop is not permitted
  end
  puts needs_pin

  T.reveal_type(pinned) # error: `T.nilable(T.type_parameter(:U) (of Object#example))`
  y
end
