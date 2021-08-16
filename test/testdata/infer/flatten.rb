# typed: true

flat_tuple = T.let(
  [1, 2],
  [Integer, Integer],
)
nested_tuple = T.let(
  [1, [2, 3]],
  [Integer, [Integer, Integer]],
)

xs = T.let(
  [1],
  T::Array[Integer],
)
xss = T.let(
  [[1]],
  T::Array[T::Array[Integer]],
)
xsss = T.let(
  [[[1]]],
  T::Array[T::Array[T::Array[Integer]]],
)

class IntegerPair
  extend T::Sig

  sig { params(left: Integer, right: Integer).void }
  def initialize(left, right)
    @left = left
    @right = right
  end

  sig { returns(T::Array[Integer]) }
  def to_ary
    [@left, @right]
  end
end

class SuperPair < IntegerPair
end

class GenericPair
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig { params(left: Elem, right: Elem).void }
  def initialize(left, right)
    @left = left
    @right = right
  end

  sig { returns(T::Array[Elem]) }
  def to_ary
    [@left, @right]
  end
end

integer_pairs = T.let([IntegerPair.new(1, 2), IntegerPair.new(3, 4)], T::Array[IntegerPair])
super_pairs = T.let([SuperPair.new(1, 2)], T::Array[SuperPair])

generic_pairs = T.let([GenericPair.new(1, 2), GenericPair.new(3, 4)], T::Array[GenericPair[Integer]])
nested_generic_pairs = T.let(
  [GenericPair.new(GenericPair.new(1, 2), GenericPair.new(3, 4))],
  T::Array[GenericPair[GenericPair[Integer]]]
)

T.reveal_type(flat_tuple.flatten) # error: Revealed type: `T::Array[Integer]`
T.reveal_type(nested_tuple.flatten) # error: Revealed type: `T::Array[Integer]`

T.reveal_type(xs.flatten) # error: Revealed type: `T::Array[Integer]`
T.reveal_type(xss.flatten) # error: Revealed type: `T::Array[Integer]`

T.reveal_type(xss.flatten(0)) # error: Revealed type: `T::Array[T::Array[Integer]]`
T.reveal_type(xss.flatten(1)) # error: Revealed type: `T::Array[Integer]`
T.reveal_type(xss.flatten(2)) # error: Revealed type: `T::Array[Integer]`

T.reveal_type(xsss.flatten(-1)) # error: Revealed type: `T::Array[Integer]`
T.reveal_type(xsss.flatten(0)) # error: Revealed type: `T::Array[T::Array[T::Array[Integer]]]`
T.reveal_type(xsss.flatten(1)) # error: Revealed type: `T::Array[T::Array[Integer]]`
T.reveal_type(xsss.flatten(2)) # error: Revealed type: `T::Array[Integer]`

T.reveal_type(integer_pairs.flatten) # error: Revealed type: `T::Array[Integer]`
T.reveal_type(super_pairs.flatten) # error: Revealed type: `T::Array[Integer]`
T.reveal_type(generic_pairs.flatten) # error: Revealed type: `T::Array[Integer]`
T.reveal_type(nested_generic_pairs.flatten) # error: Revealed type: `T::Array[Integer]`
T.reveal_type(nested_generic_pairs.flatten(1)) # error: Revealed type: `T::Array[T::Array[GenericPair[Integer]]]`

xs.flatten(1 + 1) # error: You must pass an Integer literal to specify a depth

xs.flatten(true)
#          ^^^^ error: Expected `Integer` but found `TrueClass` for argument `depth`
#          ^^^^ error: You must pass an Integer literal to specify a depth with Array#flatten

  xs.flatten(1, 1)
# ^^^^^^^^^^^^^^^^ error: Too many arguments provided for method `Array#flatten`. Expected: `0..1`, got: `2`
