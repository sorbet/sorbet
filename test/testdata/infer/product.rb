# typed: true

foo = T.let(
  ["a", "b", "c"],
  T::Array[String]
)

bar = T.let(
  [1, 2, 3],
  T::Array[Integer]
)

mixed_type = T.let(
  [1, "alpha", 0.0],
  T::Array[T.any(String, Integer, Float)]
)

empty = T.let(
  [],
  T::Array[T.untyped]
)


T.assert_type!(empty.product, T::Array[T.untyped])
T.assert_type!(foo.product, T::Array[[String]])
T.assert_type!(foo.product(mixed_type), T::Array[[String, T.any(String, Integer, Float)]])
T.assert_type!(foo.product(bar), T::Array[[String, Integer]])
T.assert_type!(foo.product(bar, foo), T::Array[[String, Integer, String]])
