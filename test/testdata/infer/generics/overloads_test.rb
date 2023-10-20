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

sig do
  params(
    type: T::Class[Integer],
    args: T.untyped,
    blk: T.nilable(T.proc.params(arg0: Integer).void)
  )
  .void
end
sig do
   params(
      args: T.untyped,
      blk: T.untyped
    )
    .void
end
def self.example2(type=T.unsafe(nil), *args, &blk)
end

example2(Integer, "--jobs=<n>") do |x|
  T.reveal_type(x) # error: `Integer`
end
example2(Integer, "-j<n>", "--jobs=<n>") do |x|
  # We pick the wrong overload, because our arity is weird in guessOverload
  T.reveal_type(x) # error: `T.untyped`
end
example2("foo", /bar/) do |x|
  #      ^^^^^ error: Expected `T::Class[Integer]` but found `String("foo")` for argument `type`
  T.reveal_type(x) # => `Integer`
end

