# typed: strict
extend T::Sig

sig do
  params(
    ints: T::Enumerable[Integer],
    floats: T::Enumerable[Float],
    hash: T::Hash[Symbol, Integer],
  ).void
end
def example(ints, floats, hash)
  res = ints.sum
  T.reveal_type(res) # error: `Integer`

  res = floats.sum
  T.reveal_type(res) # error: `Float`

  1.times do
    res = hash.sum # error: This code is unreachable
  end

  1.times do
    res = ('a'..'z').sum # error: This code is unreachable
  end

  hash.sum {|k, v| k * v}
  #                  ^ error: Method `*` does not exist on `Symbol`

  res = hash.sum do |k, v|
    k.length.to_f * v
  end
  T.reveal_type(res) # error: `Float`

  # There's no way to know, in general, what the type of calling `+` on the
  # initial value will be, so we have to say `T.untyped`.
  res = ints.sum(0.0)
  T.reveal_type(res) # error: `T.untyped`

  res = ints.sum('')
  T.reveal_type(res) # error: `T.untyped`

  res = hash.sum([])
  T.reveal_type(res) # error: `T.untyped`

  res = hash.sum(T::Array[String].new)
  T.reveal_type(res) # error: `T.untyped`

  res = hash.sum([]) do |pair|
    T.reveal_type(pair) # error: `[Symbol, Integer] (2-tuple)`
    pair
  end
  T.reveal_type(res) # error: `T.untyped`

  res = "a\nb\nc".each_line.lazy.map(&:chomp).sum("")
  T.reveal_type(res) # error: `T.untyped`
end

class A
  extend T::Generic
  extend T::Sig
  include Enumerable

  Elem = type_member {{fixed: String}}

  sig {override.params(blk: T.proc.params(arg0: Elem).returns(BasicObject)).returns(T.untyped)}
  def each(&blk)
    yield 'one'
    yield 'two'
  end
end

1.times do
  res = A.new.sum # error: This code is unreachable
end
