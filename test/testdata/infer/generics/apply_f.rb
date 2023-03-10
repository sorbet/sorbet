# typed: true
extend T::Sig

sig do
  type_parameters(:U)
    .params(
      x: T.type_parameter(:U),
      f: T.proc.params(x: T.type_parameter(:U)).returns(T.type_parameter(:U))
    )
    .returns(T.type_parameter(:U))
end
def apply_f(x, &f)
  yield x
end

sig do
  type_parameters(:U)
    .params(
      x: T.all(T.type_parameter(:U), Integer),
      f: T.proc.params(x: T.type_parameter(:U)).returns(T.type_parameter(:U))
    )
    .returns(T.type_parameter(:U))
end
def apply_f_int(x, &f)
  yield x
end

res = apply_f_int(0) do |x|
  T.reveal_type(x) # error: `Integer`
end
T.reveal_type(res) # error: `Integer`

sig {params(int: Integer).void}
def example(int)
  x = apply_f(int) do |x|
    T.reveal_type(x) # error: `Integer`
  end
  T.reveal_type(x) # error: `Integer`

  x = apply_f_int(int) do |x|
    T.reveal_type(x) # error: `Integer`
  end
  T.reveal_type(x) # error: `Integer`
end

sig {params(y: T.any(Integer, String)).void}
def takes_any_int_string(y)
  x = apply_f(y) do |x|
    T.reveal_type(x) # error: `T.any(Integer, String)`
  end
  T.reveal_type(x) # error: `T.any(Integer, String)`

  x = apply_f_int(y) do |x|
    #             ^ error: Expected `T.all(Integer, T.type_parameter(:U))` but found `T.any(Integer, String)` for argument `x`
    T.reveal_type(x) # error: `Integer`
  end
  T.reveal_type(x) # error: `Integer`
end
