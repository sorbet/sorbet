# typed: strict
extend T::Sig

sig {params(x: T.unknown).returns(T.unknown)}
def example(x)
  x.nil?
  # ^^^^ error: Method `nil?` does not exist on `T.unknown`
  T.reveal_type(x) # error: `T.unknown`
  x
end

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .returns(T.type_parameter(:U))
end
def id(x)
  res = example(x) # this is ok
  res
# ^^^ error: Expected `T.type_parameter(:U) (of Object#id)` but found `T.unknown` for method result type
end

xs = T::Array[T.unknown].new
T.reveal_type(xs) # error: `T::Array[T.unknown]`
