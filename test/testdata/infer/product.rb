# typed: true

arr_string = T.let(["a", "b", "c"], T::Array[String])
arr_integer = T.let([1, 2, 3], T::Array[Integer])
mixed_type = T.let([1, "alpha", 0.0], T::Array[T.any(String, Integer, Float)])
homogeneous_tuple_type = T.let(["foo", "bar"], [String, String])
heterogeneous_tuple_type = T.let(["testing", 123], [String, Integer])
empty = T.let([], T::Array[T.untyped])

T.reveal_type(empty.product) # error: Revealed type: `T::Array[[T.untyped]]`
T.reveal_type(arr_string.product) # error: Revealed type: `T::Array[[String]]`

T.reveal_type(empty.product(homogeneous_tuple_type)) # error: `T::Array[[T.untyped, String]]`
T.reveal_type(empty.product(heterogeneous_tuple_type)) # error: `T::Array[[T.untyped, T.any(String, Integer)]]`

T.reveal_type(arr_string.product(mixed_type)) # error: `T::Array[[String, T.any(String, Integer, Float)]]`

T.reveal_type(arr_string.product(arr_integer)) # error: `T::Array[[String, Integer]]`
T.reveal_type(arr_string.product(arr_integer, arr_string)) # error: `T::Array[[String, Integer, String]]`
T.reveal_type(arr_string.product(arr_string, arr_string, arr_string, arr_string)) # error: `T::Array[[String, String, String, String, String]]`

T.reveal_type(empty.product(arr_integer)) # error: `T::Array[[T.untyped, Integer]]`

arr_string.product("haha") # error: Expected `T::Array[T.untyped]` but found `String("haha")` for argument `arg`
arr_string.product(arr_integer, "haha") # error: Expected `T::Array[T.untyped]` but found `String("haha")` for argument `arg`
