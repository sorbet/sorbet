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

homogeneous_tuple_type = T.let(
  ["foo", "bar"],
  [String, String]
)

heterogeneous_tuple_type = T.let(
  ["testing", 123],
  [String, Integer]
)

empty = T.let(
  [],
  T::Array[T.untyped]
)


T.assert_type!(empty.product, T::Array[T.untyped])
T.assert_type!(foo.product, T::Array[[String]])

T.assert_type!(empty.product(homogeneous_tuple_type), T::Array[[T.untyped, String]])
T.assert_type!(
  empty.product(homogeneous_tuple_type),
  T::Array[[T.untyped, T.any(String, Integer)]]
)

T.assert_type!(
  foo.product(mixed_type),
  T::Array[[String, T.any(String, Integer, Float)]]
)

T.assert_type!(foo.product(bar), T::Array[[String, Integer]])
T.assert_type!(foo.product(bar, foo), T::Array[[String, Integer, String]])
T.assert_type!(
  foo.product(foo, foo, foo, foo),
  T::Array[[String, String, String, String, String]]
)

T.assert_type!(empty.product(bar), T::Array[[T.untyped, Integer]])

foo.product("haha") # error: Expected `T::Array[U]` but found `String("haha")` for argument `arg0`
          # ^^^^^^ error: You must pass an Array as every argument to Array#product

foo.product(bar, "haha") # error: Expected `T::Array[U]` but found `String("haha")` for argument `arg0`
               # ^^^^^^ error: You must pass an Array as every argument to Array#product
