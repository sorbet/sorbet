# typed: true
extend T::Sig

sig do
  type_parameters(:U)
  .params(x: T.type_parameter(:U))
  .returns(T.type_parameter(:U))
end
def fake_identity_function(x)
  case x
  when Integer
    T.reveal_type(x) # error: `T.all(Integer, T.type_parameter(:U) (of Object#fake_identity_function))`
    return 0 # error: Expected `T.type_parameter(:U) (of Object#fake_identity_function)` but found `Integer(0)` for method result type
  else
    T.reveal_type(x) # error: `T.type_parameter(:U) (of Object#fake_identity_function)`
    return x
  end
end
