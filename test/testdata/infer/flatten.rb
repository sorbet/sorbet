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

T.assert_type!(flat_tuple.flatten, T::Array[Integer]);
T.assert_type!(nested_tuple.flatten, T::Array[Integer])

T.assert_type!(xs.flatten, T::Array[Integer])
T.assert_type!(xss.flatten, T::Array[Integer])

T.assert_type!(xss.flatten(0), T::Array[T::Array[Integer]])
T.assert_type!(xss.flatten(1), T::Array[Integer])
T.assert_type!(xss.flatten(2), T::Array[Integer])

T.assert_type!(xsss.flatten(-1), T::Array[Integer])
T.assert_type!(xsss.flatten(0), T::Array[T::Array[T::Array[Integer]]])
T.assert_type!(xsss.flatten(1), T::Array[T::Array[Integer]])
T.assert_type!(xsss.flatten(2), T::Array[Integer])

xs.flatten(1 + 1) # error: You must pass an Integer literal to specify a depth

xs.flatten(true) # error: `TrueClass` doesn't match `Integer` for argument `depth`
         # ^^^^ error: You must pass an Integer literal to specify a depth with Array#flatten
