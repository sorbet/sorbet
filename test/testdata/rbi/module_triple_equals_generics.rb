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
    T.reveal_type(x) # error: `T.all(Integer, Object#fake_identity_function#U)`
    return 0 # error: Expected `Object#fake_identity_function#U` but found `Integer(0)` for method result type
  else
    T.reveal_type(x) # error: `Object#fake_identity_function#U`
    return x
  end
end
