# typed: strong
extend T::Sig

sig do
  type_parameters(:Return)
    .params(blk: T::Proc[T.type_parameter(:Return)])
    .returns(T.type_parameter(:Return))
end
def example(&blk)
  res = yield(x: 0)
  T.reveal_type(res)
  res
end

res = example do |x:|
  T.reveal_type(x)
  0
end
T.reveal_type(res)

res = T::Proc[Integer].new do |x:|
  0
end

T.reveal_type(res)

f = T.let(->() { 0 }, T.proc.returns(Integer))
T.let(f, T::Proc[Integer])
T.let(f, T::Proc[String])
