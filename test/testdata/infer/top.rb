# typed: strict
extend T::Sig

sig {params(x: T.anything).returns(T.anything)}
def example(x)
  x.nil?
  # ^^^^ error: Method `nil?` does not exist on `T.anything`
  T.reveal_type(x) # error: `T.anything`
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
# ^^^ error: Expected `T.type_parameter(:U) (of Object#id)` but found `T.anything` for method result type
end

xs = T::Array[T.anything].new
T.reveal_type(xs) # error: `T::Array[T.anything]`
