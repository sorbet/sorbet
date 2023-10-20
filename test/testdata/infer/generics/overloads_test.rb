# typed: strict
class Module; include T::Sig; end

sig do
  type_parameters(:U)
    .params(x: T::Array[T.type_parameter(:U)])
    .returns(T.nilable(T.type_parameter(:U)))
end
sig do
  params(x: String)
    .returns(T.nilable(String))
end
def example(x)
end

xs = T::Array[Integer].new

res = example(xs)
T.reveal_type(res) # error: `T.nilable(Integer)`

res = example('')
T.reveal_type(res) # error: `T.nilable(String)`
