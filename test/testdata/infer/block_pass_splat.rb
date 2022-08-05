# typed: true
extend T::Sig

sig {params(xs: T::Array[Integer]).void}
def example1(xs)
  res = xs.map(&:even?)
  T.reveal_type(res) # error: `T::Array[T::Boolean]`

  xs.map(&:even)
  #       ^^^^^ error: Method `even` does not exist on `Integer`

  res = xs.reduce(0, &:+)
  T.reveal_type(res) # error: `Integer`
end

sig {params(xs: T::Array[T.nilable(Integer)]).void}
def example2(xs)
  xs.map(&:even?)
  #       ^^^^^^ error: Method `even?` does not exist on `NilClass` component of `T.nilable(Integer)`
end

class A
  extend T::Sig
  sig {params(x: String).void}
  def requires_keyword(x:)
  end
  sig {params(x: String).void}
  def optional_keyword(x: '')
  end

  sig {params(blk: T.proc.params(a: A).void).void}
  def self.example1(&blk)
    yield A.new
  end

  # Not currently possible to say that a block will be yielded a keyword arg
  sig {params(blk: T.proc.params(a: A, x: Integer).void).void}
  def self.example2(&blk)
    yield A.new, x: 0 # error: Expected `Integer` but found `{x: Integer(0)}` for argument `arg1`
  end

  # Can somewhat fake it with shape types
  sig {params(blk: T.proc.params(a: A, kwargs: {x: Integer}).void).void}
  def self.example3(&blk)
    yield A.new, x: 0
  end
end

A.example1(&:requires_keyword)
#                            ^ error: Missing required keyword argument `x` for method `A#requires_keyword`
A.example2(&:requires_keyword)
#           ^ error: Missing required keyword argument `x` for method `A#requires_keyword`
#           ^ error: Too many positional arguments provided for method `A#requires_keyword`. Expected: `0`, got: `1`
A.example3(&:requires_keyword)
#           ^^^^^^^^^^^^^^^^^ error: Expected `String` but found `Integer` for argument `x`

A.example1(&:optional_keyword)
A.example2(&:optional_keyword)
#           ^ error: Too many positional arguments provided for method `A#optional_keyword`. Expected: `0`, got: `1`
A.example3(&:optional_keyword)
#           ^^^^^^^^^^^^^^^^^ error: Expected `String` but found `Integer` for argument `x`
