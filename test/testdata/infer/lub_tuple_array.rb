# typed: true
extend T::Sig

sig {
  type_parameters(:U)
  .params(
    x: T.any(
      T.type_parameter(:U),
      T::Array[T.type_parameter(:U)]
    )
  )
  .returns(T::Array[T.type_parameter(:U)])
}
def foo(x)
  res = case x
  when Array
    # This type is particularly unwieldy, and I'd love to make it smaller if possible.
    # But this test used to crash us, and at least it doesn't do that anymore.
    T.reveal_type(x) # error: `T.all(T::Array[T.untyped], T.any(T::Array[T.type_parameter(:U) (of Object#foo)], T.type_parameter(:U) (of Object#foo)))`

  else
    T.reveal_type(x) # error: `T.type_parameter(:U) (of Object#foo)`
    [x]
  end

  T.reveal_type(res) # error: `T::Array[T.type_parameter(:U) (of Object#foo)]`
end
