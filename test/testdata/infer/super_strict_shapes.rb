# typed: true

extend T::Sig

xs = {
  foo: T.let(nil, T.nilable(Integer)),
  bar: nil,
}

T.reveal_type(xs) # error: Revealed type: `{foo: T.nilable(Integer), bar: NilClass} (shape of T::Hash[Symbol, T.nilable(Integer)])`

xs[:foo] = 1
xs[:bar] = 1 # error: Expected `NilClass` but found `Integer(1)` for key `Symbol(:bar)`
xs[:qux] # error: Key `Symbol(:qux)` not present in shape `{foo: T.nilable(Integer), bar: NilClass}`

options = {
  enable_foo: T.let(false, T::Boolean),
  enable_bar: T.let(false, T::Boolean),
  enable_qux: false,
}

T.reveal_type(options) # error: Revealed type: `{enable_foo: T::Boolean, enable_bar: T::Boolean, enable_qux: FalseClass} (shape of T::Hash[Symbol, T::Boolean])`

ARGV.each do |arg|
  case arg
  when 'enable_foo' then options[:enable_foo] = true
  when 'enable_bar' then options[:enable_bar] = true
  when 'enable_qux' then options[:enable_qux] = true # error: Expected `FalseClass` but found `TrueClass` for key `Symbol(:enable_qux)`
  end
end

T.reveal_type(options) # error: Revealed type: `{enable_foo: T::Boolean, enable_bar: T::Boolean, enable_qux: FalseClass} (shape of T::Hash[Symbol, T::Boolean])`

# These merge algorithms more or less assume exact shape types.
#
# Otherwise, if you were assuming inexact, you'd have to widen the keys of the
# first shape (like `:one` and `:three`) because if the second shape is
# inexact, you don't know that it doesn't also have a `:one` or `:three` key
# with a completely unrelated type.

evens = {
  one: 1,
  three: 3,
}
odds = {
  two: 2,
  four: 4,
}
T.reveal_type(evens.merge(odds)) # error: Revealed type: `{one: Integer(1), three: Integer(3), two: Integer(2), four: Integer(4)} (shape of T::Hash[Symbol, Integer])`

# Looks like our Shape_merge intrinsic hardcodes one positional argument
T.reveal_type({}.merge(evens, odds)) # error: Revealed type: `{} (shape of T::Hash[T.untyped, T.untyped])`

one_three = {
  one: 1,
  three: 3,
}
three_five = {
  three: '3',
  five: '5',
}
T.reveal_type(one_three.merge(three_five)) # error: Revealed type: `{one: Integer(1), three: String("3"), five: String("5")} (shape of T::Hash[Symbol, T.any(Integer, String)])`

# Fixes https://github.com/sorbet/sorbet/issues/3751
sig {params(xs: T::Array[Integer]).void}
def test1(xs = [])
  T.reveal_type(xs) # error: Revealed type: `T::Array[Integer]`
end

sig {params(x: T::Array[String]).void}
def test2(x)
  x = []
  T.reveal_type(x) # error: Revealed type: `[] (0-tuple)`

  if T.unsafe(nil)
    y = []
    z = T::Array[Integer].new
  else
    y = T::Array[Integer].new
    z = []
  end
  T.reveal_type(y) # error: Revealed type: `T::Array[Integer]`
  T.reveal_type(z) # error: Revealed type: `T::Array[Integer]`
end
